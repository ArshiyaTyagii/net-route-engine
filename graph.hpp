// graph.hpp
// Weighted, undirected-by-default network topology graph backed by an
// adjacency list. Nodes are identified by string labels (router/host
// names), which keeps the CLI and network files human-readable.
//
// Design notes
// ------------
// * Adjacency list chosen over adjacency matrix: real network topologies
//   are sparse (a router has a handful of links, not O(N) links to every
//   other router), so adjacency lists give O(V + E) space instead of
//   O(V^2), and O(deg(v)) neighbor iteration instead of O(V).
// * Node labels are interned to small integer indices internally so that
//   Dijkstra/Bellman-Ford/Kruskal can work over dense integer ranges
//   (contiguous vectors, no hashing in the hot path).

#pragma once

#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace netroute {

struct Edge {
    int to{};
    int weight{};
};

class Graph {
public:
    static constexpr int kInvalidIndex = -1;

    // Returns the internal index for `label`, creating a new node if it
    // does not already exist.
    int addNode(const std::string& label);

    // Returns the internal index for `label`, or kInvalidIndex if unknown.
    [[nodiscard]] int indexOf(const std::string& label) const;

    [[nodiscard]] const std::string& labelOf(int index) const;

    // Adds a weighted edge between two labels. If `directed` is false
    // (default), the reverse edge is added automatically, modeling a
    // bidirectional network link.
    void addEdge(const std::string& from, const std::string& to, int weight,
                 bool directed = false);

    [[nodiscard]] std::size_t nodeCount() const noexcept { return labels_.size(); }
    [[nodiscard]] std::size_t edgeCount() const noexcept { return edgeCount_; }

    [[nodiscard]] const std::vector<Edge>& neighbors(int nodeIndex) const {
        return adjacency_[static_cast<std::size_t>(nodeIndex)];
    }

    [[nodiscard]] const std::vector<std::string>& labels() const noexcept { return labels_; }

    // Returns all edges as (from, to, weight) triples, each undirected
    // edge reported once (from < to) to avoid double-processing in
    // algorithms like Kruskal's MST that operate on an edge list.
    struct WeightedEdge {
        int from;
        int to;
        int weight;
    };
    [[nodiscard]] std::vector<WeightedEdge> allEdgesUnique() const;

    static constexpr int kInfinity = std::numeric_limits<int>::max();

private:
    std::vector<std::string> labels_;
    std::unordered_map<std::string, int> labelToIndex_;
    std::vector<std::vector<Edge>> adjacency_;
    std::size_t edgeCount_{0};
};

} // namespace netroute
