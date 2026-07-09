// test_union_find.cpp
#include "netroute/union_find.hpp"
#include "test_framework.hpp"

using netroute::UnionFind;

TEST_CASE(union_find_starts_with_all_singletons) {
    UnionFind dsu(5);
    for (int i = 0; i < 5; ++i) {
        REQUIRE_EQ(dsu.find(i), i);
    }
}

TEST_CASE(union_find_unite_merges_sets) {
    UnionFind dsu(4);
    REQUIRE(dsu.unite(0, 1));
    REQUIRE(dsu.connected(0, 1));
    REQUIRE(!dsu.connected(0, 2));
}

TEST_CASE(union_find_unite_returns_false_for_already_connected) {
    UnionFind dsu(3);
    REQUIRE(dsu.unite(0, 1));
    REQUIRE(!dsu.unite(0, 1)); // already in the same set -- would form a cycle
}

TEST_CASE(union_find_transitive_connectivity) {
    UnionFind dsu(5);
    dsu.unite(0, 1);
    dsu.unite(1, 2);
    REQUIRE(dsu.connected(0, 2)); // 0-1-2 chain implies 0 and 2 are connected
    REQUIRE(!dsu.connected(0, 3));
}
