#include "strategy.hpp"
#include <iostream>

void MSTStrategy::execute(Graph& graph) const {
    std::cout << "MST Weight: " << graph.mstWeight() << std::endl; //print mst weight
}

void CliqueStrategy::execute(Graph& graph) const {
    std::cout << "Total Cliques: " << graph.countCliques() << std::endl; //print total cliques
} 

void SCCStrategy::execute(Graph& graph) const {
    std::cout << "Strongly Connected Components:\n"; //print scc header
    auto sccs = graph.findSCCs(); //get scc list
    for (const auto& component : sccs) {
        for (int v : component)
            std::cout << v << " "; //print each vertex in component
        std::cout << "\n"; //new line after each component
    }
}

void MaxFlowStrategy::execute(Graph& graph) const {
    int maxFlowValue = graph.maxFlow(0, graph.getNumVertices() - 1); //calculate max flow
    std::cout << "Max Flow from 0 to " << graph.getNumVertices()-1 << ": " << maxFlowValue << std::endl; //print result
}
