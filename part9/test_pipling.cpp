#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "pipling.hpp"
#include "graph.hpp"

#include <algorithm>
#include <vector>
#include <chrono>


static void sort_components(std::vector<std::vector<int>>& comps) {
    for (auto& c : comps) std::sort(c.begin(), c.end());
    std::sort(comps.begin(), comps.end());
}


TEST_CASE("Pipling: start → submit one UNDIRECTED path graph → get() returns full result, then stop") {
    Graph g(4,false);
    g.addEdge(0,1);
    g.addEdge(1,2);
    g.addEdge(2,3);

    Pipling p;
    p.start();
    p.submit(g);
    Pipling::Result r = p.get();  // blocks until all 4 stages complete
    p.stop();
    CHECK(r.mst_weight == 3);
    CHECK(r.num_cliques == 7);

    auto comps = r.sccs;
    sort_components(comps);
    REQUIRE(comps.size() == 1);
    CHECK(comps[0] == std::vector<int>({0,1,2,3}));
    CHECK(r.max_flow == 1);
}

TEST_CASE("Pipling: two jobs back-to-back (UNDIRECTED path + DIRECTED two-parallel-paths), order preserved") {
    Graph und(4, false);
    und.addEdge(0,1);
    und.addEdge(1,2);
    und.addEdge(2,3);

    Graph dir(4, true);
    dir.addEdge(0,1);
    dir.addEdge(1,3);
    dir.addEdge(0,2);
    dir.addEdge(2,3);

    Pipling p;
    p.start();
    p.submit(und);
    p.submit(dir);

    Pipling::Result r1 = p.get();
    Pipling::Result r2 = p.get();

    p.stop();
    CHECK(r1.mst_weight == 3);
    CHECK(r1.max_flow == 1);
    CHECK(r2.mst_weight == -1);
    CHECK(r2.max_flow == 2);

    auto comps_dir = r2.sccs;
    sort_components(comps_dir);
    REQUIRE(comps_dir.size() == 4);
    CHECK(comps_dir[0].size() == 1);
    CHECK(comps_dir[1].size() == 1);
    CHECK(comps_dir[2].size() == 1);
    CHECK(comps_dir[3].size() == 1);
    CHECK(r2.num_cliques == 8);
}

TEST_CASE("Pipling: stop() with no jobs — starts threads and shuts down cleanly via sentinel chain") {
    Pipling p;
    p.start();
    // No submit()
    p.stop(); // should not hang
    CHECK(true);
}

TEST_CASE("Pipling: destructor calls stop() — no deadlock even without jobs") {
    // Enter scope to force destructor
    {
        Pipling p;
        p.start();
        // no submit(), leave scope → ~Pipling() calls stop()
    }
    CHECK(true);
}
