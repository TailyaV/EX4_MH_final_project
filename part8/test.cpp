#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "graph.hpp"
#include <vector>

static std::set<std::pair<int,int>> edge_set_undirected(const Graph& g) {
    std::set<std::pair<int,int>> es;
    int V = g.getNumVertices();
    for (int u = 0; u < V; ++u) {
        const auto& Nu = g.getNeighbors(u);
        for (int v : Nu) {
            if (u < v) es.emplace(u, v);
        }
    }
    return es;
}

TEST_CASE("addEdge"){
    Graph g(4, false);
    g.addEdge(1,2);
    CHECK_THROWS_AS_MESSAGE(g.addEdge(0,5),std::invalid_argument , "Error: Invalid vertex index.\n");
    CHECK_THROWS_AS_MESSAGE(g.addEdge(1,1),std::invalid_argument , "Error: Invalid vertex index.\n");
    CHECK_EQ(g.getNeighbors(1).front(), 2);
    CHECK_EQ(g.getNeighbors(2).front(), 1);
    Graph g2(4, true);
    g2.addEdge(1,2);
    g2.addEdge(2,3);
    CHECK_EQ(g2.getNeighbors(1).front(), 2);
    CHECK_NE(g2.getNeighbors(2).front(), 1);
}

TEST_CASE("removeEdge"){
    Graph g(4, false);
    g.addEdge(1,2);
    g.addEdge(1,3);
    g.addEdge(0,2);
    g.removeEdge(2,3);
    CHECK_THROWS_AS_MESSAGE(g.removeEdge(0,5),std::invalid_argument , "Error: Invalid vertex index.\n");
    CHECK_THROWS_AS_MESSAGE(g.removeEdge(1,1),std::invalid_argument , "Error: Invalid vertex index.\n");
    g.removeEdge(1,2);
    CHECK_EQ(g.getNeighbors(1).front(), 3);
    CHECK_EQ(g.getNeighbors(2).front(), 0);
    Graph g2(4, true);
    g2.addEdge(1,2);
    g2.addEdge(2,1);
    g2.removeEdge(2,1);
    CHECK_EQ(g2.getNeighbors(1).front(), 2);
}

TEST_CASE("printGraph") {
    Graph g(3, false);
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    std::ostringstream buffer;
    std::streambuf* oldCoutBuf = std::cout.rdbuf(buffer.rdbuf());
    g.printGraph(); 
    std::cout.rdbuf(oldCoutBuf);
    std::string expected =
        "0: 1 \n"
        "1: 0 2 \n"
        "2: 1 \n";
    CHECK(buffer.str() == expected);

    Graph g2(3, true);
    g2.addEdge(0, 1);
    g2.addEdge(1, 2);
    std::ostringstream buffer2;
    std::streambuf* oldCoutBuf2 = std::cout.rdbuf(buffer2.rdbuf());
    g2.printGraph(); 
    std::cout.rdbuf(oldCoutBuf2);
    std::string expected2 =
        "0: 1 \n"
        "1: 2 \n"
        "2: \n";
    CHECK(buffer2.str() == expected2);
}

TEST_CASE("getNeighbors"){
    Graph g(4, false);
    g.addEdge(0,1);
    g.addEdge(2,0);
    vector<int> expect;
    expect.push_back(1);
    expect.push_back(2);
    CHECK_THROWS_AS_MESSAGE(g.getNeighbors(5),std::invalid_argument , "Error: Invalid vertex index.\n");
    CHECK(g.getNeighbors(0) == expect);
}

TEST_CASE("getNumVertices"){
    Graph g(8, false);
    CHECK_EQ(g.getNumVertices(), 8);
}

TEST_CASE("getNumVertices"){
    Graph g(4, false);
    g.addEdge(0,1);
    g.addEdge(1,2);
    g.addEdge(2,3);
    g.addEdge(3,0);
    CHECK_EQ(g.hasEulerCircuit(), true);
    Graph g2(4, true);
    g2.addEdge(0,1);
    g2.addEdge(1,2);
    g2.addEdge(2,3);
    g2.addEdge(0,3);
    CHECK_EQ(g2.hasEulerCircuit(), false);
    Graph g3(4, false);
    CHECK_EQ(g3.hasEulerCircuit(), false);
}

TEST_CASE("findEulerCircuit"){
    Graph g(4, false);
    g.addEdge(0,1);
    g.addEdge(1,2);
    g.addEdge(2,3);
    g.addEdge(3,0);
    vector<int> expect = {0,1,2,3};
    CHECK(g.findEulerCircuit().front() == expect.front());
    Graph g2(4, false);
    g2.addEdge(0,1);
    vector<int> expect2 = {};
    CHECK(g2.findEulerCircuit() == expect2);
    Graph g3(4, true);
    g3.addEdge(0,1);
    g3.addEdge(1,2);
    g3.addEdge(2,3);
    g3.addEdge(3,0);
    CHECK(g3.findEulerCircuit().at(2) == expect.at(2));
}

TEST_CASE("removeAllEdges"){
    Graph g(4, false);
    g.addEdge(0,1);
    g.addEdge(1,2);
    g.addEdge(2,3);
    g.addEdge(3,0);
    g.addEdge(3,1);
    vector<int> expect = {};
    g.removeAllEdges();
    CHECK(g.getNeighbors(0) == expect);
    CHECK(g.getNeighbors(1) == expect);
    CHECK(g.getNeighbors(2) == expect);
    CHECK(g.getNeighbors(3) == expect);
}

//Halper for order components
static void sort_components(std::vector<std::vector<int>>& comps) {
    for (auto& c : comps) std::sort(c.begin(), c.end());
    std::sort(comps.begin(), comps.end());
}

TEST_CASE("mstWeight") {
    Graph g(4, false);
    g.addEdge(0,1);
    g.addEdge(1,2);
    g.addEdge(2,3);
    CHECK(g.mstWeight() == 3);
}

TEST_CASE("mstWeight") {
    Graph g(5, false);
    g.addEdge(0,1);
    g.addEdge(0,2);
    g.addEdge(0,3);
    g.addEdge(0,4);
    CHECK(g.mstWeight() == 4);
}

TEST_CASE("mstWeight") {
    Graph g(1, false);
    CHECK(g.mstWeight() == 0);
}

TEST_CASE("mstWeight") {
    Graph g(3, true);
    g.addEdge(0,1);
    g.addEdge(1,2);
    CHECK(g.mstWeight() == -1);
}

TEST_CASE("countCliques") {
    Graph g(3, false);
    CHECK(g.countCliques() == 3);
}

TEST_CASE("countCliques") {
    Graph g(3, false);
    g.addEdge(0,1);
    g.addEdge(0,2);
    g.addEdge(1,2);
    CHECK(g.countCliques() == 7);
}

TEST_CASE("countCliques") {
    Graph g(3, false);
    g.addEdge(0,1);
    g.addEdge(1,2);
    CHECK(g.countCliques() == 5);
}

TEST_CASE("findSCCs") {
    Graph g(3, true);
    g.addEdge(0,1);
    g.addEdge(1,2);
    g.addEdge(2,0);
    auto comps = g.findSCCs();
    sort_components(comps);
    REQUIRE(comps.size() == 1);
    CHECK(comps[0] == std::vector<int>({0,1,2}));
}

TEST_CASE("findSCCs") {
    Graph g(4, true);
    g.addEdge(0,1);
    g.addEdge(1,0);
    g.addEdge(2,3);
    g.addEdge(3,2);
    g.addEdge(1,2);
    auto comps = g.findSCCs();
    sort_components(comps);
    REQUIRE(comps.size() == 2);
    CHECK(comps[0] == std::vector<int>({0,1}));
    CHECK(comps[1] == std::vector<int>({2,3}));
}

TEST_CASE("maxFlow") {
    Graph g(4, true);
    g.addEdge(0,1);
    g.addEdge(1,3);
    g.addEdge(0,2);
    g.addEdge(2,3);
    CHECK(g.maxFlow(0,3) == 2);
}

TEST_CASE("maxFlow: no path, flow 0") {
    Graph g(3, true);
    g.addEdge(0,1);
    CHECK(g.maxFlow(0,2) == 0);
}

TEST_CASE("maxFlow") {
    Graph g(4, true);
    g.addEdge(0,1);
    g.addEdge(0,2);
    g.addEdge(1,2);
    g.addEdge(1,3);
    g.addEdge(2,3);
    CHECK(g.maxFlow(0,3) == 2);
}


TEST_CASE("buildRandGraph: throws when requested edges exceed maximum for V") {
    CHECK_THROWS_AS(Graph::buildRandGraph(7, 4, 123), std::invalid_argument);
}


TEST_CASE("buildRandGraph: zero edges yields empty adjacency") {
    int V = 5, E = 0, seed = 42;
    Graph g = Graph::buildRandGraph(E, V, seed);
    CHECK(g.getNumVertices() == V);
    for (int u = 0; u < V; ++u) {
        CHECK(g.getNeighbors(u).empty());
    }
}

TEST_CASE("buildRandGraph: structure & determinism (same seed gives same graph)") {
    int V = 6, E = 7, seed = 1337;

    Graph g1 = Graph::buildRandGraph(E, V, seed);
    Graph g2 = Graph::buildRandGraph(E, V, seed);
    CHECK(g1.getNumVertices() == V);
    CHECK(g2.getNumVertices() == V);
    auto es1 = edge_set_undirected(g1);
    auto es2 = edge_set_undirected(g2);
    CHECK(es1.size() == static_cast<size_t>(E));
    CHECK(es2.size() == static_cast<size_t>(E));
    CHECK(es1 == es2);

    for (auto [u, v] : es1) {
        const auto& Nu = g1.getNeighbors(u);
        const auto& Nv = g1.getNeighbors(v);
        CHECK(std::find(Nu.begin(), Nu.end(), v) != Nu.end());
        CHECK(std::find(Nv.begin(), Nv.end(), u) != Nv.end());
        CHECK(u != v); 
    }
}
