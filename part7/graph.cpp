#include "graph.hpp"           
#include <iostream>       
#include <algorithm>   
#include <stack>
#include <queue>
#include <limits.h>

//constructor for the Graph class
Graph::Graph(int vertices, bool directed) 
    : numVertices(vertices),  
      directed(directed),        
      adjList(vertices) {}       

//function to add an edge between two vertices u and v
void Graph::addEdge(int u, int v) {
    if (u < 0 || u >= numVertices || v < 0 || v >= numVertices) {  //check if u and v are valid indices
        std::cout << "Error: Invalid vertex index." << std::endl;  //print error if they are out of range
        return;                                                   
    }
    adjList[u].push_back(v); //add v to u's adjacency list
    if (!directed) { //if the graph is undirected
        adjList[v].push_back(u);//add u to v's adjacency list as well
    }
}

//function to remove an edge between two vertices u and v
void Graph::removeEdge(int u, int v) {
    if (u < 0 || u >= numVertices || v < 0 || v >= numVertices) {  //check if u and v are valid indices
        std::cout << "Error: Invalid vertex index." << std::endl;  
        return;                                                
    }
    auto& neighborsU = adjList[u]; //reference to u's adjacency list
    neighborsU.erase(                                              
        std::remove(neighborsU.begin(), neighborsU.end(), v), //find v in u's list and move it to the end
        neighborsU.end() //erase the found element(s)
    );
    if (!directed) {  //if the graph is undirected
        auto& neighborsV = adjList[v]; //reference to v's adjacency list
        neighborsV.erase(                                          
            std::remove(neighborsV.begin(), neighborsV.end(), u),  //find u in v's list and move it to the end
            neighborsV.end() //erase the found element(s)
        );
    }
}

//function to print the adjacency list of the graph
void Graph::printGraph() const {
    for (int i = 0; i < numVertices; i++) {       
        std::cout << i << ": "; //print the current vertex index
        for (int neighbor : adjList[i]) {         
            std::cout << neighbor << " "; //print each neighbor
        }
        std::cout << std::endl; //move to a new line after printing all neighbors
    }
}

//function to get the list of neighbors for vertex v
const std::vector<int>& Graph::getNeighbors(int v) const {
    return adjList[v];         
}

//function to get the total number of vertices in the graph
int Graph::getNumVertices() const {
    return numVertices;                        
}

//depth-First Search helper function to mark all reachable vertices
void Graph::dfs(int v, vector<bool>& visited, const vector<vector<int>>& localAdjList) const {
    visited[v] = true; //mark the current vertex as visited

    //loop over all neighbors of vertex v
    for (int neighbor : localAdjList[v]) {
        //if the neighbor hasn't been visited yet, visit it recursively
        if (!visited[neighbor]) {
            dfs(neighbor, visited, localAdjList);
        }
    }
}

//checks whether the graph contains an Euler circuit
bool Graph::hasEulerCircuit() const {
    vector<bool> visited(numVertices, false);//create a visited array initialized to false
    int start = -1;

    //find a vertex with at least one edge to start the DFS from
    for (int i = 0; i < numVertices; ++i) {
        if (!adjList[i].empty()) { //this vertex has neighbors
            start = i;
            break;
        }
    }
    //if no vertex has any edges, the graph is empty (no edges), which is a trivial Euler circuit
    if (start == -1) return true; //no edges

    //perform DFS to check if all non-isolated vertices are reachable (i.e., graph is connected)
    dfs(start, visited, adjList);

    //ensure all vertices that have edges are visited
    for (int i = 0; i < numVertices; ++i) {
        if (!adjList[i].empty() && !visited[i]) return false;//graph is not connected
    }

    //check that all vertices have even degree
    for (const auto& neighbors : adjList) {
        if (neighbors.size() % 2 != 0) return false;//vertex has odd degree =no Euler circuit

    }
    return true;
}

//finds and returns an Euler circuit starting from the given vertex
vector<int> Graph::findEulerCircuit(int start) {
    vector<int> circuit;

    //if the graph does not have an Euler circuit, return an empty vector
    if (!hasEulerCircuit()) return circuit;

    vector<vector<int>> tempAdj = adjList;//make a temporary copy of the adjacency list to modify
    stack<int> path;//stack to track the current path in the traversal
    vector<int> result;//to store the final Euler circuit in reverse

    path.push(start);//begin traversal at the starting vertex

    while (!path.empty()) {
        int v = path.top();//look at the current vertex on top of the stack
        if (!tempAdj[v].empty()) {
            //while there are unused edges from v, go deeper
            int u = tempAdj[v].back();//choose the last neighbor
            tempAdj[v].pop_back(); //remove edge vu from v’s list
            if (!directed) {
                //for undirected graph, also remove the edge u → v from u's list
                auto& vec = tempAdj[u];
                vec.erase(remove(vec.begin(), vec.end(), v), vec.end());
            }
            path.push(u);//continue to the neighbor u
        } else {
            //no more edges left from v → backtrack
            result.push_back(v);//add v to the result
            path.pop();//go back to previous vertex
        }
    }
    reverse(result.begin(), result.end());//reverse the result to get correct path order
    return result;//return the final Euler circuit
}




// ---------- Minimum Spanning Tree (Prim's) ----------
int Graph::mstWeight() const {
    if (directed) return -1; //MST only works on undirected graphs

    int totalWeight = 0;
    std::vector<bool> visited(numVertices, false);
    std::vector<int> minEdge(numVertices, INT_MAX);  //store min edge weight for each vertex
    minEdge[0] = 0;  //start from vertex 0

    for (int i = 0; i < numVertices; ++i) {
        int u = -1;

        //find unvisited vertex with the smallest edge weight
        for (int v = 0; v < numVertices; ++v) {
            if (!visited[v] && (u == -1 || minEdge[v] < minEdge[u])) {
                u = v;
            }
        }

        visited[u] = true;
        totalWeight += minEdge[u];

        //update min edges for neighbors
        for (int neighbor : adjList[u]) {
            if (!visited[neighbor]) {
                minEdge[neighbor] = std::min(minEdge[neighbor], 1); // All edges have weight 1
            }
        }
    }

    return totalWeight;
}

// ---------- Counting Cliques (brute force) ----------
bool isClique(const std::vector<int>& subset, const std::vector<std::vector<int>>& adj) {
    //check every pair in subset: must be connected both ways
    for (size_t i = 0; i < subset.size(); ++i) {
        for (size_t j = i + 1; j < subset.size(); ++j) {
            int u = subset[i], v = subset[j];
            if (std::find(adj[u].begin(), adj[u].end(), v) == adj[u].end())
                return false;
        }
    }
    return true;
}

int Graph::countCliques() {
    int count = 0;
    int n = numVertices;

    //go over all subsets using bitmask
    for (int mask = 1; mask < (1 << n); ++mask) {
        std::vector<int> subset;
        for (int i = 0; i < n; ++i)
            if (mask & (1 << i))
                subset.push_back(i);
        if (isClique(subset, adjList)) count++;
    }

    return count;
}

// ---------- Strongly Connected Components (Kosaraju) ----------

//first DFS to fill the order stack
void dfs1(int v, std::vector<bool>& visited, std::stack<int>& order, const std::vector<std::vector<int>>& adj) {
    visited[v] = true;
    for (int u : adj[v])
        if (!visited[u]) dfs1(u, visited, order, adj);
    order.push(v);
}

//second DFS on the transposed graph
void dfs2(int v, std::vector<bool>& visited, std::vector<int>& component, const std::vector<std::vector<int>>& revAdj) {
    visited[v] = true;
    component.push_back(v);
    for (int u : revAdj[v])
        if (!visited[u]) dfs2(u, visited, component, revAdj);
}

std::vector<std::vector<int>> Graph::findSCCs() {
    std::stack<int> order;
    std::vector<bool> visited(numVertices, false);

    //fill order of nodes based on finishing times
    for (int i = 0; i < numVertices; ++i)
        if (!visited[i]) dfs1(i, visited, order, adjList);

    //build reverse graph
    std::vector<std::vector<int>> revAdj(numVertices);
    for (int u = 0; u < numVertices; ++u)
        for (int v : adjList[u])
            revAdj[v].push_back(u);

    std::fill(visited.begin(), visited.end(), false);
    std::vector<std::vector<int>> components;

    //process vertices in reverse finishing order
    while (!order.empty()) {
        int v = order.top(); order.pop();
        if (!visited[v]) {
            std::vector<int> component;
            dfs2(v, visited, component, revAdj);
            components.push_back(component);
        }
    }

    return components;
}

// ---------- Max Flow (Edmonds-Karp) ----------

//BFS to find an augmenting path
bool bfsFlow(const std::vector<std::vector<int>>& rGraph, int s, int t, std::vector<int>& parent) {
    int n = rGraph.size();
    std::vector<bool> visited(n, false);
    std::queue<int> q;

    q.push(s);
    visited[s] = true;
    parent[s] = -1;

    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v = 0; v < n; ++v) {
            if (!visited[v] && rGraph[u][v] > 0) {
                q.push(v);
                parent[v] = u;
                visited[v] = true;
            }
        }
    }

    return visited[t]; //return true if sink is reachable
}

int Graph::maxFlow(int source, int sink) {
    //initialize residual graph
    std::vector<std::vector<int>> rGraph(numVertices, std::vector<int>(numVertices, 0));
    for (int u = 0; u < numVertices; ++u)
        for (int v : adjList[u])
            rGraph[u][v] += 1; //edge capacity is 1

    std::vector<int> parent(numVertices);
    int max_flow = 0;

    //while there's an augmenting path
    while (bfsFlow(rGraph, source, sink, parent)) {
        int path_flow = INT_MAX;

        //find minimum capacity in the path
        for (int v = sink; v != source; v = parent[v])
            path_flow = std::min(path_flow, rGraph[parent[v]][v]);

        //update residual capacities
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            rGraph[u][v] -= path_flow;
            rGraph[v][u] += path_flow;
        }

        max_flow += path_flow;
    }

    return max_flow;
}
