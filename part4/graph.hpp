#pragma once
#define GRAPH_H
#include <vector>
using namespace std;

class Graph {
private:
    int numVertices; // Number of vertices in the graph
    bool directed;  // Whether the graph is directed
    vector<vector<int>> adjList; // Adjacency list: adjList[u] contains neighbors of u
    void dfs(int v, vector<bool>& visited, const vector<vector<int>>& localAdjList) const;  // DFS used for connectivity check (used in hasEulerCircuit)

public:
    Graph(int vertices, bool directed = false); // Constructor
    void addEdge(int u, int v); // Add edge between u and v
    void removeEdge(int u, int v); // Remove edge between u and v
    void removeAllEdges(); //Remove all edges from the graph
    void printGraph() const; // Print adjacency list

    const vector<int>& getNeighbors(int v) const; // Return neighbors of vertex v
    int getNumVertices() const;  // Return total vertices

    bool hasEulerCircuit() const;// Check if Euler circuit exists
    vector<int> findEulerCircuit(int start = 0); // Return Euler circuit starting from given vertex
};