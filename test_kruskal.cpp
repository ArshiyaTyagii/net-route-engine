// test_kruskal.cpp
#include "netroute/kruskal.hpp"
#include "test_framework.hpp"

using netroute::Graph;
using netroute::kruskalMST;

TEST_CASE(kruskal_finds_minimum_spanning_tree) {
    Graph g;
    g.addEdge("A", "B", 1);
    g.addEdge("B", "C", 2);
    g.addEdge("A", "C", 3); // would create a cycle if included with the above two
    g.addEdge("C", "D", 4);

    const auto mst = kruskalMST(g);
    REQUIRE(mst.spansAllNodes);
    REQUIRE_EQ(mst.edges.size(), static_cast<std::size_t>(3)); // V - 1 = 3 edges for 4 nodes
    REQUIRE_EQ(mst.totalWeight, 7LL); // 1 + 2 + 4, excluding the redundant A-C(3)
}

TEST_CASE(kruskal_disconnected_graph_reports_incomplete_span) {
    Graph g;
    g.addEdge("A", "B", 1);
    g.addEdge("X", "Y", 1); // separate component, unreachable from A/B

    const auto mst = kruskalMST(g);
    REQUIRE(!mst.spansAllNodes);
    REQUIRE_EQ(mst.edges.size(), static_cast<std::size_t>(2)); // one edge per component
}

TEST_CASE(kruskal_single_node_graph_is_trivially_spanning) {
    Graph g;
    g.addNode("Solo");
    const auto mst = kruskalMST(g);
    REQUIRE(mst.spansAllNodes);
    REQUIRE(mst.edges.empty());
    REQUIRE_EQ(mst.totalWeight, 0LL);
}

TEST_CASE(kruskal_ignores_redundant_parallel_higher_weight_edge) {
    Graph g;
    g.addEdge("A", "B", 5);
    g.addEdge("A", "B", 2); // cheaper parallel link should still be considered

    const auto mst = kruskalMST(g);
    REQUIRE_EQ(mst.edges.size(), static_cast<std::size_t>(1));
    // Kruskal sorts by weight globally, so the first A-B edge processed
    // is the lightest one available for that pair.
    REQUIRE_EQ(mst.totalWeight, 2LL);
}
