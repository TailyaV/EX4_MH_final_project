#include "graph.hpp"
#include "factory.hpp"
#include "strategy.hpp"
#include <iostream>
#include <memory>

//builds a sample undirected graph with some cycles and edges
void buildSampleGraph(Graph& g) {
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 0);  //triangle (clique)
    g.addEdge(2, 3);
    g.addEdge(3, 4);
    g.addEdge(4, 0);  //another cycle
    g.addEdge(1, 4);  //extra edge
    g.addEdge(0, 3);
}

//tests various public methods of the Graph class
void testAllGraphFunctions(Graph& g) {
    std::cout << "\n== Testing Graph API ==\n";

    g.printGraph(); //print adjacency list

    std::cout << "Removing edge 0-1...\n";
    g.removeEdge(0, 1); //remove edge

    std::cout << "Neighbors of node 2:\n";
    const auto& neighbors = g.getNeighbors(2); //get neighbors of vertex 2
    for (int n : neighbors) std::cout << n << " ";
    std::cout << "\n";

    std::cout << "Checking for Euler Circuit: " 
              << (g.hasEulerCircuit() ? "Yes" : "No") << "\n"; //check euler circuit

    std::cout << "Trying to find Euler Circuit from node 0:\n";
    auto circuit = g.findEulerCircuit(0); //get euler circuit path
    for (int v : circuit) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "Testing invalid edge inputs:\n";
    g.addEdge(-1, 100); //invalid edge
    g.removeEdge(999, 1); //invalid remove
}

int main() {
    Graph g(5, false); //undirected graph with 5 nodes
    buildSampleGraph(g);
    testAllGraphFunctions(g);

    std::cout << "\n== Running Algorithms via Factory ==\n";
    for (int algoID = MST; algoID <= MAX_FLOW; ++algoID) {
        auto algo = GraphAlgorithmFactory::create(static_cast<AlgorithmType>(algoID)); //create strategy
        if (algo) {
            std::cout << "\nAlgorithm " << algoID << ":\n";
            algo->execute(g); //run strategy
        } else {
            std::cerr << "Factory failed to create algorithm for ID " << algoID << "\n"; //error if creation fails
        }
    }

    return 0;
}
