#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
using namespace std;

class Graph {
private:
    int numVertices; //number of vertices in the graph
    bool directed;  // whether the graph is directed
    vector<vector<int>> adjList; //adjacency list: adjList[u] contains neighbors of u
    void dfs(int v, vector<bool>& visited, const vector<vector<int>>& localAdjList) const;  //DFS used for connectivity check (used in hasEulerCircuit)

public:
    Graph(int vertices, bool directed = false); //constructor
    void addEdge(int u, int v); //add edge between u and v
    void removeEdge(int u, int v); //remove edge between u and v
    void printGraph() const; //print adjacency list

    const vector<int>& getNeighbors(int v) const; //return neighbors of vertex v
    int getNumVertices() const;  //return total vertices

    bool hasEulerCircuit() const;//check if Euler circuit exists
    vector<int> findEulerCircuit(int start = 0); //return Euler circuit starting from given vertex

    //algorithm declarations
    int mstWeight() const;
    int countCliques();
    std::vector<std::vector<int>> findSCCs();
    int maxFlow(int source, int sink);
};

#endif
