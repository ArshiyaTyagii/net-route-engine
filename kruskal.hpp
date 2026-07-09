// kruskal.hpp
// Kruskal's Minimum Spanning Tree algorithm: sort all edges by weight,
// then greedily add each edge that does not form a cycle (checked via
// Union-Find). Produces the lowest-total-cost backbone that keeps every
// router reachable -- the classic use case being redundant-link network
// design where you want the cheapest set of links that still connects
// every site.
//
// Complexity: O(E log E) for the sort, O(E * alpha(V)) for the union-find
// operations, where alpha is the (effectively constant) inverse Ackermann
// function.

#pragma once

#include <vector>

#include "netroute/graph.hpp"

namespace netroute {

struct MstResult {
    std::vector<Graph::WeightedEdge> edges;
    long long totalWeight{0};
    bool spansAllNodes{false}; // false if the input graph is disconnected
};

MstResult kruskalMST(const Graph& graph);

} // namespace netroute
