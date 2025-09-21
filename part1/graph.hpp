#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
using namespace std;

class Graph {
private:
    int numVertices; // Number of vertices in the graph
    bool directed;  // Whether the graph is directed
    vector<vector<int>> adjList; // Adjacency list: adjList[u] contains neighbors of u

public:
    Graph(int vertices, bool directed = false); // Constructor
    void addEdge(int u, int v); // Add edge between u and v
    void removeEdge(int u, int v); // Remove edge between u and v
    void printGraph() const; // Print adjacency list
    const vector<int>& getNeighbors(int v) const; // Return neighbors of vertex v
    int getNumVertices() const;  // Return total vertices
};

#endif
