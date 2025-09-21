#include "graph.hpp"           
#include <iostream>       
#include <algorithm>   
#include <stack>

// Constructor for the Graph class
Graph::Graph(int vertices, bool directed) 
    : numVertices(vertices),  
      directed(directed),        
      adjList(vertices) {}       

// Function to add an edge between two vertices u and v
void Graph::addEdge(int u, int v) {
    if (u < 0 || u >= numVertices || v < 0 || v >= numVertices) {  // Check if u and v are valid indices
        std::cout << "Error: Invalid vertex index." << std::endl;  // Print error if they are out of range
        return;                                                   
    }
    adjList[u].push_back(v); // Add v to u's adjacency list
    if (!directed) { // If the graph is undirected
        adjList[v].push_back(u);// Add u to v's adjacency list as well
    }
}

// Function to remove an edge between two vertices u and v
void Graph::removeEdge(int u, int v) {
    if (u < 0 || u >= numVertices || v < 0 || v >= numVertices) {  // Check if u and v are valid indices
        std::cout << "Error: Invalid vertex index." << std::endl;  
        return;                                                
    }
    auto& neighborsU = adjList[u]; // Reference to u's adjacency list
    neighborsU.erase(                                              
        std::remove(neighborsU.begin(), neighborsU.end(), v), // Find v in u's list and move it to the end
        neighborsU.end() // Erase the found element(s)
    );
    if (!directed) {  // If the graph is undirected
        auto& neighborsV = adjList[v]; // Reference to v's adjacency list
        neighborsV.erase(                                          
            std::remove(neighborsV.begin(), neighborsV.end(), u),  // Find u in v's list and move it to the end
            neighborsV.end() // Erase the found element(s)
        );
    }
}

// Function to print the adjacency list of the graph
void Graph::printGraph() const {
    for (int i = 0; i < numVertices; i++) {       
        std::cout << i << ": "; // Print the current vertex index
        for (int neighbor : adjList[i]) {         
            std::cout << neighbor << " "; // Print each neighbor
        }
        std::cout << std::endl; // Move to a new line after printing all neighbors
    }
}

// Function to get the list of neighbors for vertex v
const std::vector<int>& Graph::getNeighbors(int v) const {
    return adjList[v];         
}

// Function to get the total number of vertices in the graph
int Graph::getNumVertices() const {
    return numVertices;                        
}

// Depth-First Search helper function to mark all reachable vertices
void Graph::dfs(int v, vector<bool>& visited, const vector<vector<int>>& localAdjList) const {
    visited[v] = true; // Mark the current vertex as visited

    // Loop over all neighbors of vertex v
    for (int neighbor : localAdjList[v]) {
        // If the neighbor hasn't been visited yet, visit it recursively
        if (!visited[neighbor]) {
            dfs(neighbor, visited, localAdjList);
        }
    }
}

// Checks whether the graph contains an Euler circuit
bool Graph::hasEulerCircuit() const {
    vector<bool> visited(numVertices, false);// Create a visited array initialized to false
    int start = -1;

    // Find a vertex with at least one edge to start the DFS from
    for (int i = 0; i < numVertices; ++i) {
        if (!adjList[i].empty()) { // This vertex has neighbors
            start = i;
            break;
        }
    }
    // If no vertex has any edges, the graph is empty (no edges), which is a trivial Euler circuit
    if (start == -1) return true; // No edges

    // Perform DFS to check if all non-isolated vertices are reachable (i.e., graph is connected)
    dfs(start, visited, adjList);

    // Ensure all vertices that have edges are visited
    for (int i = 0; i < numVertices; ++i) {
        if (!adjList[i].empty() && !visited[i]) return false;// Graph is not connected
    }

    // Check that all vertices have even degree
    for (const auto& neighbors : adjList) {
        if (neighbors.size() % 2 != 0) return false;// Vertex has odd degree =no Euler circuit

    }
    return true;
}

// Finds and returns an Euler circuit starting from the given vertex
vector<int> Graph::findEulerCircuit(int start) {
    vector<int> circuit;

    // If the graph does not have an Euler circuit, return an empty vector
    if (!hasEulerCircuit()) return circuit;

    vector<vector<int>> tempAdj = adjList;// Make a temporary copy of the adjacency list to modify
    stack<int> path;// Stack to track the current path in the traversal
    vector<int> result;// To store the final Euler circuit in reverse

    path.push(start);// Begin traversal at the starting vertex

    while (!path.empty()) {
        int v = path.top();// Look at the current vertex on top of the stack
        if (!tempAdj[v].empty()) {
            // While there are unused edges from v, go deeper
            int u = tempAdj[v].back();// Choose the last neighbor
            tempAdj[v].pop_back(); // Remove edge vu from v’s list
            if (!directed) {
                // For undirected graph, also remove the edge u → v from u's list
                auto& vec = tempAdj[u];
                vec.erase(remove(vec.begin(), vec.end(), v), vec.end());
            }
            path.push(u);// Continue to the neighbor u
        } else {
            // No more edges left from v → backtrack
            result.push_back(v);// Add v to the result
            path.pop();// Go back to previous vertex
        }
    }
    reverse(result.begin(), result.end());// Reverse the result to get correct path order
    return result;// Return the final Euler circuit
}

void Graph::removeAllEdges(){
    for (auto &inner : adjList) {
        inner.clear();
    }
    adjList.clear();
}
