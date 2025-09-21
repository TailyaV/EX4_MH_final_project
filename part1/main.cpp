#include "graph.hpp"
#include <iostream>

int main() {
    Graph g(5, false);  //Graph with 5 vertices, undirected

    //Valid edges
    g.addEdge(0, 1);
    g.addEdge(0, 4);
    g.addEdge(1, 2);
    g.addEdge(1, 3);
    g.addEdge(1, 4);
    g.addEdge(2, 3);
    g.addEdge(3, 4);

    std::cout << "Graph adjacency list:" << std::endl;
    g.printGraph();

    std::cout << "\nRemoving edge 1-4...\n";
    g.removeEdge(1, 4);

    std::cout << "\nTrying to remove invalid edge 10-2...\n";
    g.removeEdge(10, 2);  //hit the error line

    std::cout << "\nTrying to add invalid edge -1-2...\n";
    g.addEdge(-1, 2);  //hit the error line

    std::cout << "\nCalling getNeighbors(0):\n";
    for (int neighbor : g.getNeighbors(0)) {
        std::cout << neighbor << " ";
    }
    std::cout << "\n";

    std::cout << "Number of vertices: " << g.getNumVertices() << std::endl;

    return 0;
}
