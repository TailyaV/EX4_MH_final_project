#ifndef STRATEGY_H
#define STRATEGY_H

#include "graph.hpp"
#include <memory>

//base interface for all graph algorithms
class IGraphAlgorithm {
public:
    virtual ~IGraphAlgorithm() = default; //virtual destructor
    virtual void execute(Graph& graph) const = 0; //pure virtual method
};

//strategy for minimum spanning tree
class MSTStrategy : public IGraphAlgorithm {
public:
    void execute(Graph& graph) const override;
};

//strategy for counting cliques
class CliqueStrategy : public IGraphAlgorithm {
public:
    void execute(Graph& graph) const override;
};

//strategy for strongly connected components
class SCCStrategy : public IGraphAlgorithm {
public:
    void execute(Graph& graph) const override;
};

//strategy for maximum flow
class MaxFlowStrategy : public IGraphAlgorithm {
public:
    void execute(Graph& graph) const override;
};

#endif
