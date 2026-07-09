// union_find.hpp
// Disjoint Set Union (a.k.a. Union-Find) with union-by-rank and path
// compression, giving near-O(1) amortized find()/unite() -- the standard
// building block for Kruskal's MST cycle detection.

#pragma once

#include <numeric>
#include <vector>

namespace netroute {

class UnionFind {
public:
    explicit UnionFind(std::size_t n) : parent_(n), rank_(n, 0) {
        std::iota(parent_.begin(), parent_.end(), 0);
    }

    // Path-compressed find: flattens the tree on every lookup so future
    // find() calls on the same subtree are O(1).
    int find(int x) {
        while (parent_[static_cast<std::size_t>(x)] != x) {
            // Path halving: point each node at its grandparent.
            parent_[static_cast<std::size_t>(x)] =
                parent_[static_cast<std::size_t>(parent_[static_cast<std::size_t>(x)])];
            x = parent_[static_cast<std::size_t>(x)];
        }
        return x;
    }

    // Unites the sets containing a and b. Returns false if they were
    // already in the same set (i.e. adding this edge would form a cycle).
    bool unite(int a, int b) {
        int rootA = find(a);
        int rootB = find(b);
        if (rootA == rootB) {
            return false;
        }
        if (rank_[static_cast<std::size_t>(rootA)] < rank_[static_cast<std::size_t>(rootB)]) {
            std::swap(rootA, rootB);
        }
        parent_[static_cast<std::size_t>(rootB)] = rootA;
        if (rank_[static_cast<std::size_t>(rootA)] == rank_[static_cast<std::size_t>(rootB)]) {
            ++rank_[static_cast<std::size_t>(rootA)];
        }
        return true;
    }

    [[nodiscard]] bool connected(int a, int b) { return find(a) == find(b); }

private:
    std::vector<int> parent_;
    std::vector<int> rank_;
};

} // namespace netroute
