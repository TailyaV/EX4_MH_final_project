#pragma once
#include "graph.hpp"

void printCircuit(const vector<int>& circuit); // Helper to print a circuit
Graph buildRandGraph(int numOfEdges, int numOfVartx, int seed); //Build graph with random edges according to a given number ef edges and vertices