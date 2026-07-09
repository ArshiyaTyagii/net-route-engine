// shortest_path.hpp
// Single-source shortest-path algorithms over netroute::Graph.
//
// Two algorithms are provided deliberately, not redundantly:
//   * Dijkstra   - O((V + E) log V) with a binary heap; the right choice
//                  for real routing tables where link costs are always
//                  non-negative. Rejects negative weights explicitly
//                  rather than silently producing wrong answers.
//   * Bellman-Ford - O(V * E); slower, but correct in the presence of
//                  negative edge weights and able to detect negative
//                  cycles, which matter for certain path-vector/policy
//                  routing scenarios (e.g. BGP-style cost adjustments).

#pragma once

#include <optional>
#include <stdexcept>
#include <vector>

#include "netroute/graph.hpp"

namespace netroute {

struct ShortestPathResult {
    std::vector<int> distance;   // distance[i] = cost from source to node i
    std::vector<int> predecessor; // predecessor[i] = previous node on the
                                   // shortest path, or Graph::kInvalidIndex
    bool negativeCycleDetected{false}; // only ever set by Bellman-Ford

    // Reconstructs the path from source to `target` as a sequence of node
    // indices. Returns std::nullopt if target is unreachable.
    [[nodiscard]] std::optional<std::vector<int>> pathTo(int target) const;
};

class NegativeWeightError : public std::domain_error {
public:
    explicit NegativeWeightError(const std::string& what) : std::domain_error(what) {}
};

// Requires all edge weights >= 0; throws NegativeWeightError otherwise.
ShortestPathResult dijkstra(const Graph& graph, int source);

// Tolerates negative edge weights. If a negative cycle reachable from the
// source exists, result.negativeCycleDetected is set to true and the
// distance array should not be trusted for nodes affected by the cycle.
ShortestPathResult bellmanFord(const Graph& graph, int source);

} // namespace netroute
