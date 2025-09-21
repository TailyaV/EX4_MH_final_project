#ifndef FACTORY_H
#define FACTORY_H

#include "strategy.hpp"
#include <memory>

//enum to represent available graph algorithm types
enum AlgorithmType {
    MST = 1,
    CLIQUES,
    SCC,
    MAX_FLOW
};

//factory class to create algorithm objects based on the selected type
class GraphAlgorithmFactory {
public:
    //returns a specific algorithm strategy based on the type given
    static std::unique_ptr<IGraphAlgorithm> create(AlgorithmType type);
};

#endif
