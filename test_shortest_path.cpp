// test_shortest_path.cpp
#include "netroute/shortest_path.hpp"
#include "test_framework.hpp"

using netroute::bellmanFord;
using netroute::dijkstra;
using netroute::Graph;
using netroute::NegativeWeightError;

TEST_CASE(dijkstra_finds_shortest_path_in_simple_graph) {
    Graph g;
    g.addEdge("A", "B", 4);
    g.addEdge("A", "C", 1);
    g.addEdge("C", "B", 1);
    g.addEdge("B", "D", 1);

    const auto result = dijkstra(g, g.indexOf("A"));
    // A -> C -> B -> D = 1 + 1 + 1 = 3, cheaper than direct A -> B -> D = 5
    REQUIRE_EQ(result.distance[static_cast<std::size_t>(g.indexOf("D"))], 3);

    const auto path = result.pathTo(g.indexOf("D"));
    REQUIRE(path.has_value());
    REQUIRE_EQ(path->size(), static_cast<std::size_t>(4)); // A, C, B, D
}

TEST_CASE(dijkstra_rejects_negative_weights) {
    Graph g;
    g.addEdge("A", "B", -1);
    REQUIRE_THROWS_AS(dijkstra(g, g.indexOf("A")), NegativeWeightError);
}

TEST_CASE(dijkstra_unreachable_node_reports_no_path) {
    Graph g;
    g.addNode("A");
    g.addNode("Isolated");
    g.addEdge("A", "B", 1);

    const auto result = dijkstra(g, g.indexOf("A"));
    const auto path = result.pathTo(g.indexOf("Isolated"));
    REQUIRE(!path.has_value());
}

TEST_CASE(bellman_ford_handles_negative_weights) {
    Graph g;
    g.addEdge("A", "B", 4, /*directed=*/true);
    g.addEdge("A", "C", 5, /*directed=*/true);
    g.addEdge("C", "B", -2, /*directed=*/true);

    const auto result = bellmanFord(g, g.indexOf("A"));
    REQUIRE(!result.negativeCycleDetected);
    // A -> C -> B = 5 + (-2) = 3, cheaper than direct A -> B = 4
    REQUIRE_EQ(result.distance[static_cast<std::size_t>(g.indexOf("B"))], 3);
}

TEST_CASE(bellman_ford_detects_negative_cycle) {
    Graph g;
    g.addEdge("A", "B", 1, /*directed=*/true);
    g.addEdge("B", "C", -3, /*directed=*/true);
    g.addEdge("C", "A", 1, /*directed=*/true);

    const auto result = bellmanFord(g, g.indexOf("A"));
    REQUIRE(result.negativeCycleDetected);
}

TEST_CASE(graph_undirected_edge_is_traversable_both_ways) {
    Graph g;
    g.addEdge("A", "B", 7);
    const auto result = dijkstra(g, g.indexOf("B"));
    REQUIRE_EQ(result.distance[static_cast<std::size_t>(g.indexOf("A"))], 7);
}

TEST_CASE(graph_disconnected_components_stay_unreachable) {
    Graph g;
    g.addEdge("A", "B", 1);
    g.addEdge("X", "Y", 1); // separate component

    const auto result = dijkstra(g, g.indexOf("A"));
    REQUIRE(!result.pathTo(g.indexOf("X")).has_value());
    REQUIRE(!result.pathTo(g.indexOf("Y")).has_value());
}
