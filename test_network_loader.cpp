// test_network_loader.cpp
#include "netroute/network_loader.hpp"
#include "test_framework.hpp"

#include <cstdio>
#include <fstream>

using netroute::loadNetworkFile;
using netroute::NetworkFileError;

namespace {

// Writes `contents` to a temp file and returns its path. Using a fixed
// name under the test working directory keeps this dependency-free.
std::string writeTempFile(const std::string& name, const std::string& contents) {
    const std::string path = "test_tmp_" + name;
    std::ofstream out(path);
    out << contents;
    out.close();
    return path;
}

} // namespace

TEST_CASE(network_loader_parses_links_and_routes) {
    const std::string path = writeTempFile("valid.txt",
        "# sample network\n"
        "LINK RouterA RouterB 4\n"
        "LINK RouterB RouterC 2\n"
        "ROUTE 192.168.1.0/24 RouterA 1\n");

    const auto net = loadNetworkFile(path);
    REQUIRE_EQ(net.topology.nodeCount(), static_cast<std::size_t>(3));
    REQUIRE_EQ(net.topology.edgeCount(), static_cast<std::size_t>(2));
    REQUIRE_EQ(net.routingTable.size(), static_cast<std::size_t>(1));

    std::remove(path.c_str());
}

TEST_CASE(network_loader_ignores_blank_lines_and_full_line_comments) {
    const std::string path = writeTempFile("comments.txt",
        "\n"
        "# just a comment\n"
        "LINK A B 1\n"
        "\n");
    const auto net = loadNetworkFile(path);
    REQUIRE_EQ(net.topology.edgeCount(), static_cast<std::size_t>(1));
    std::remove(path.c_str());
}

TEST_CASE(network_loader_rejects_unknown_directive) {
    const std::string path = writeTempFile("bad_directive.txt", "TELEPORT A B\n");
    REQUIRE_THROWS_AS(loadNetworkFile(path), NetworkFileError);
    std::remove(path.c_str());
}

TEST_CASE(network_loader_rejects_malformed_link_arity) {
    const std::string path = writeTempFile("bad_link.txt", "LINK OnlyOneNode\n");
    REQUIRE_THROWS_AS(loadNetworkFile(path), NetworkFileError);
    std::remove(path.c_str());
}

TEST_CASE(network_loader_rejects_non_numeric_link_weight) {
    const std::string path = writeTempFile("bad_weight.txt", "LINK A B heavy\n");
    REQUIRE_THROWS_AS(loadNetworkFile(path), NetworkFileError);
    std::remove(path.c_str());
}

TEST_CASE(network_loader_rejects_route_missing_cidr_slash) {
    const std::string path = writeTempFile("bad_route.txt", "ROUTE 10.0.0.0 RouterA\n");
    REQUIRE_THROWS_AS(loadNetworkFile(path), NetworkFileError);
    std::remove(path.c_str());
}

TEST_CASE(network_loader_rejects_route_bad_prefix_length) {
    const std::string path = writeTempFile("bad_prefix_len.txt", "ROUTE 10.0.0.0/99 RouterA\n");
    REQUIRE_THROWS_AS(loadNetworkFile(path), NetworkFileError);
    std::remove(path.c_str());
}

TEST_CASE(network_loader_missing_file_throws_runtime_error) {
    REQUIRE_THROWS_AS(loadNetworkFile("this_file_does_not_exist.txt"), std::runtime_error);
}

TEST_CASE(network_loader_accepts_optional_route_metric) {
    const std::string path = writeTempFile("metric.txt", "ROUTE 10.0.0.0/8 RouterA 42\n");
    const auto net = loadNetworkFile(path);
    const auto match = net.routingTable.search(
        netroute::Trie::parseIPv4("10.0.0.0"), 8);
    REQUIRE(match.has_value());
    REQUIRE_EQ(match->metric, 42);
    std::remove(path.c_str());
}
