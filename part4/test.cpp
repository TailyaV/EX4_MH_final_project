#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "graph.hpp"
#include <vector>

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