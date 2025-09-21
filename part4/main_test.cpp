#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "graph.hpp"
#include "main.hpp"
#include <vector>

TEST_CASE("printCircuit"){
    Graph g(3, false);
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(0, 2);
    std::ostringstream buffer;
    std::streambuf* oldCoutBuf = std::cout.rdbuf(buffer.rdbuf());
    printCircuit(g.findEulerCircuit()); 
    std::cout.rdbuf(oldCoutBuf);
    std::string expected =
        "0 2 1 0 \n";
    CHECK(buffer.str() == expected);
}

TEST_CASE("printCircuit"){
    Graph g = buildRandGraph(4, 4, 25);
    CHECK(g.getNeighbors(0).size() + g.getNeighbors(1).size() + g.getNeighbors(2).size() + g.getNeighbors(3).size() == 8);
     Graph g2 = buildRandGraph(6, 4, 100);
    CHECK(g2.getNeighbors(0).size() + g2.getNeighbors(1).size() + g2.getNeighbors(2).size() + g2.getNeighbors(3).size() == 12);
    CHECK(g.getNumVertices() == 4);
    CHECK_THROWS_AS_MESSAGE(buildRandGraph(7, 4, 25), std::invalid_argument, "Graph with 4 vatexes cant have 7 edgaes.\n");
}