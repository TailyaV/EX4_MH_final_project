#include "factory.hpp"

//factory method that creates and returns an algorithm object based on the enum value passed in (AlgorithmType)
std::unique_ptr<IGraphAlgorithm> GraphAlgorithmFactory::create(AlgorithmType type) {
    switch (type) {
        case MST: return std::make_unique<MSTStrategy>();
        case CLIQUES: return std::make_unique<CliqueStrategy>();
        case SCC: return std::make_unique<SCCStrategy>();
        case MAX_FLOW: return std::make_unique<MaxFlowStrategy>();
        default: return nullptr; //invalid type, return null
    }
}
