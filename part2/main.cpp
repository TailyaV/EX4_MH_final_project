#include "graph.hpp"
#include <iostream>
using namespace std;

//Helper to print a circuit
void printCircuit(const vector<int>& circuit) {
    for (int v : circuit) {
        cout << v << " ";
    }
    cout << endl;
}

int main() {
    // Graph with Euler Circuit
    cout << "Test 1: Graph with Euler Circuit\n";
    Graph g1(5, false);
    g1.addEdge(0, 1);
    g1.addEdge(1, 2);
    g1.addEdge(2, 0);
    g1.addEdge(0, 3);
    g1.addEdge(3, 4);
    g1.addEdge(4, 0);

    g1.printGraph();

    if (g1.hasEulerCircuit()) {
        cout << "Euler Circuit exists. Path:\n";
        printCircuit(g1.findEulerCircuit());
    } else {
        cout << "No Euler Circuit.\n";
    }

    cout << "\n";

    //Euler Path, not a Circuit
    cout << "Test 2: Euler Path (not a circuit)\n";
    Graph g2(4, false);
    g2.addEdge(0, 1);
    g2.addEdge(1, 2);
    g2.addEdge(2, 3);

    g2.printGraph();

    if (g2.hasEulerCircuit()) {
        cout << "Euler Circuit exists. Path:\n";
        printCircuit(g2.findEulerCircuit());
    } else {
        cout << "No Euler Circuit (but this graph has an Euler path).\n";
    }

    cout << "\n";

    //Neither Euler Path nor Circuit
    cout << "Test 3: Neither Euler Path nor Circuit\n";
    Graph g3(3, false);
    g3.addEdge(0, 1);
    g3.addEdge(1, 2);
    g3.addEdge(2, 0);
    g3.addEdge(0, 1); //extra edge → odd degree

    g3.printGraph();

    if (g3.hasEulerCircuit()) {
        cout << "Euler Circuit exists. Path:\n";
        printCircuit(g3.findEulerCircuit());
    } else {
        cout << "No Euler Circuit or Path.\n";
    }

    cout << "\n";

    //Test edge cases for full coverage
    cout << "Test 4: Invalid Inputs and Removals \n";
    Graph g4(3, false);

    //Try invalid addEdge (out-of-bounds indices)
    g4.addEdge(-1, 2);   //should print error
    g4.addEdge(2, 3);    //should print error

    //Add and remove edges properly
    g4.addEdge(0, 1);
    g4.removeEdge(0, 1); //remove valid
    g4.removeEdge(2, 3); //invalid remove

    //Use getters
    cout << "Vertices count: " << g4.getNumVertices() << endl;
    const auto& neighbors = g4.getNeighbors(0);
    cout << "Neighbors of 0: ";
    for (int n : neighbors) cout << n << " ";
    cout << endl;

    return 0;
}
