#include "graph.hpp"           
#include <iostream>       
#include <algorithm>            

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
