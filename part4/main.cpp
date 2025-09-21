#include "graph.hpp"
#include <iostream>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include "main.hpp"
using namespace std;

extern char *optarg;   //holds the option argument
extern int optopt;

// Helper to print a circuit
void printCircuit(const vector<int>& circuit) {
    for (int v : circuit) {
        cout << v << " ";
    }
    cout << endl;
}

//Build graph with random edges according to a given number ef edges and vertices
Graph buildRandGraph(int numOfEdges, int numOfVartx, int seed){
    // Compute the maximum number of edges in a simple undirected graph with V vertices
    int allEdgesNum = numOfVartx*(numOfVartx - 1)/2;
    // Validate that requested number of edges does not exceed the maximum possible
    if(numOfEdges > allEdgesNum){
        throw invalid_argument("Graph with" + to_string(numOfVartx) + " vatexes cant have " + to_string(numOfEdges) + "edgaes.\n");
    }
    srand(seed);
    // Create an undirected graph
    Graph graph(numOfVartx, false);

    vector<pair<int, int>> allAdges;
    vector<int> chosen;
    // Build the list of all possible undirected edges
    for (int u = 0; u < numOfVartx; u++) {
        for (int v = u + 1; v < numOfVartx; v++) {
            allAdges.push_back({u, v});
        }
    }
    // Randomly choose 'numOfEdges' distinct edges
    for(int i = 0; i < numOfEdges; i++){
        int randNum = rand() % allEdgesNum;
        // If this index was already used, keep drawing until a new one is found
        while(find(chosen.begin(), chosen.end(), randNum) != chosen.end()){
            randNum = rand() % allEdgesNum;
        }
            // Add the corresponding edge to the graph
            graph.addEdge(allAdges[randNum].first, allAdges[randNum].second);
            // Mark the index as used
            chosen.push_back(randNum);
        
    }
    return graph;
}

#ifndef UNIT_TESTING
int main(int argc, char* argv[]) {
    //Usage check: expects -e <edges> -v <vertices> -s <seed>
    if(argc < 7){
        cout << "Please enter 'program name' -e <numberOfEdges> -v <numberOfVertax> -s <seed>" << endl;
        return 1;
    }
    int numOfEdges = 0;
    int numOfVartx = 0;
    int seed;
    int ret;

    //Get the arguments from the user and cheack correctness
    while((ret = getopt(argc, argv, ":e:v:s:")) != -1){
        switch(ret){
            //Unknown option
            case '?':
                std::cout << static_cast<char>(optopt) << " is unknown option" << std::endl;
            //No value provided to option
            case ':':
                std::cout << "No value provided to option " << static_cast<char>(optopt) << std::endl;
            case 'e':
                try{numOfEdges = stoi(optarg);
                }catch(const std::invalid_argument&){printf("invalid argument, write number(int) of edges\n"); return 1; }
            case 'v':
                try{numOfVartx = stoi(optarg);
                }catch(const std::invalid_argument&){printf("invalid argument, write number(int) of vartexes\n"); return 1; }
            case 's':
                 try{seed = stoi(optarg);
                }catch(const std::invalid_argument&){printf("invalid argument, write number(int) for seed\n"); return 1; }
        }
    }
    Graph graph = buildRandGraph(numOfEdges, numOfVartx, seed); 
    // Print the graph
    graph.printGraph();
    // Check if the graph has an Euler circuit; if so, compute and print it
    if (graph.hasEulerCircuit()) {
        cout << "Euler Circuit exists. Path:\n";
        printCircuit(graph.findEulerCircuit());
    } else {
        cout << "No Euler Circuit.\n";
    }
    
    return 0;
}
#endif