// test_trie.cpp
#include "netroute/trie.hpp"
#include "test_framework.hpp"

using netroute::InvalidPrefixError;
using netroute::RouteEntry;
using netroute::Trie;

TEST_CASE(trie_parse_and_format_ipv4_roundtrip) {
    const std::uint32_t addr = Trie::parseIPv4("192.168.1.25");
    REQUIRE_EQ(Trie::formatIPv4(addr), std::string("192.168.1.25"));
}

TEST_CASE(trie_parse_ipv4_rejects_malformed_input) {
    REQUIRE_THROWS_AS(Trie::parseIPv4("192.168.1"), InvalidPrefixError);      // too few octets
    REQUIRE_THROWS_AS(Trie::parseIPv4("192.168.1.1.5"), InvalidPrefixError);  // too many
    REQUIRE_THROWS_AS(Trie::parseIPv4("192.168.1.256"), InvalidPrefixError); // out of range
    REQUIRE_THROWS_AS(Trie::parseIPv4("abc.def.1.1"), InvalidPrefixError);   // non-numeric
    REQUIRE_THROWS_AS(Trie::parseIPv4(""), InvalidPrefixError);              // empty
    REQUIRE_THROWS_AS(Trie::parseIPv4("192..1.1"), InvalidPrefixError);      // empty octet
}

TEST_CASE(trie_insert_and_exact_search) {
    Trie trie;
    const std::uint32_t addr = Trie::parseIPv4("10.0.0.0");
    trie.insert(RouteEntry{addr, 8, "eth0", 1});

    const auto found = trie.search(addr, 8);
    REQUIRE(found.has_value());
    REQUIRE_EQ(found->nextHop, std::string("eth0"));

    // Same address, different prefix length: no exact match.
    REQUIRE(!trie.search(addr, 16).has_value());
}

TEST_CASE(trie_rejects_invalid_prefix_length) {
    Trie trie;
    const std::uint32_t addr = Trie::parseIPv4("10.0.0.0");
    REQUIRE_THROWS_AS(trie.insert(RouteEntry{addr, 33, "eth0"}), InvalidPrefixError);
    REQUIRE_THROWS_AS(trie.search(addr, 200), InvalidPrefixError);
}

TEST_CASE(trie_longest_prefix_match_prefers_most_specific) {
    Trie trie;
    trie.insert(RouteEntry{Trie::parseIPv4("192.168.0.0"), 16, "default-gw", 1});
    trie.insert(RouteEntry{Trie::parseIPv4("192.168.1.0"), 24, "specific-gw", 1});

    const auto match = trie.longestPrefixMatch(Trie::parseIPv4("192.168.1.25"));
    REQUIRE(match.has_value());
    REQUIRE_EQ(match->nextHop, std::string("specific-gw"));
    REQUIRE_EQ(static_cast<int>(match->prefixLength), 24);

    // An address outside the /24 but inside the /16 should fall back.
    const auto fallback = trie.longestPrefixMatch(Trie::parseIPv4("192.168.5.5"));
    REQUIRE(fallback.has_value());
    REQUIRE_EQ(fallback->nextHop, std::string("default-gw"));
}

TEST_CASE(trie_longest_prefix_match_default_route) {
    Trie trie;
    trie.insert(RouteEntry{0, 0, "default-route", 1}); // 0.0.0.0/0

    const auto match = trie.longestPrefixMatch(Trie::parseIPv4("8.8.8.8"));
    REQUIRE(match.has_value());
    REQUIRE_EQ(match->nextHop, std::string("default-route"));
}

TEST_CASE(trie_longest_prefix_match_no_route_returns_nullopt) {
    Trie trie;
    trie.insert(RouteEntry{Trie::parseIPv4("10.0.0.0"), 8, "eth0"});

    const auto match = trie.longestPrefixMatch(Trie::parseIPv4("172.16.0.1"));
    REQUIRE(!match.has_value());
}

TEST_CASE(trie_remove_exact_route) {
    Trie trie;
    const std::uint32_t addr = Trie::parseIPv4("10.0.0.0");
    trie.insert(RouteEntry{addr, 8, "eth0"});
    REQUIRE_EQ(trie.size(), static_cast<std::size_t>(1));

    const bool removed = trie.remove(addr, 8);
    REQUIRE(removed);
    REQUIRE_EQ(trie.size(), static_cast<std::size_t>(0));
    REQUIRE(!trie.search(addr, 8).has_value());
}

TEST_CASE(trie_remove_nonexistent_route_returns_false) {
    Trie trie;
    REQUIRE(!trie.remove(Trie::parseIPv4("1.1.1.1"), 32));
}

TEST_CASE(trie_remove_prunes_dead_branches_but_keeps_siblings) {
    Trie trie;
    trie.insert(RouteEntry{Trie::parseIPv4("10.0.0.0"), 8, "a"});
    trie.insert(RouteEntry{Trie::parseIPv4("10.1.0.0"), 16, "b"});

    trie.remove(Trie::parseIPv4("10.1.0.0"), 16);

    // The /8 route must survive removal of the unrelated /16.
    const auto stillThere = trie.search(Trie::parseIPv4("10.0.0.0"), 8);
    REQUIRE(stillThere.has_value());
    REQUIRE_EQ(stillThere->nextHop, std::string("a"));
}

TEST_CASE(trie_host_route_slash_32) {
    Trie trie;
    trie.insert(RouteEntry{Trie::parseIPv4("192.168.1.25"), 32, "host-route"});
    const auto match = trie.longestPrefixMatch(Trie::parseIPv4("192.168.1.25"));
    REQUIRE(match.has_value());
    REQUIRE_EQ(match->nextHop, std::string("host-route"));

    // A neighboring address must NOT match the /32.
    const auto noMatch = trie.longestPrefixMatch(Trie::parseIPv4("192.168.1.26"));
    REQUIRE(!noMatch.has_value());
}

TEST_CASE(trie_insert_overwrites_existing_route) {
    Trie trie;
    const std::uint32_t addr = Trie::parseIPv4("10.0.0.0");
    trie.insert(RouteEntry{addr, 8, "first"});
    trie.insert(RouteEntry{addr, 8, "second"});

    REQUIRE_EQ(trie.size(), static_cast<std::size_t>(1)); // overwrite, not duplicate
    const auto match = trie.search(addr, 8);
    REQUIRE_EQ(match->nextHop, std::string("second"));
}

TEST_CASE(trie_performance_bulk_insert_and_lookup) {
    // Not a rigorous benchmark -- just a sanity check that the trie
    // handles a few thousand routes without pathological behavior.
    Trie trie;
    constexpr int kRouteCount = 5000;
    for (int i = 0; i < kRouteCount; ++i) {
        const std::uint32_t addr = (static_cast<std::uint32_t>(10) << 24) |
                                    (static_cast<std::uint32_t>(i >> 8) << 8) |
                                    static_cast<std::uint32_t>(i & 0xFF);
        trie.insert(RouteEntry{addr, 32, "route-" + std::to_string(i)});
    }
    REQUIRE_EQ(trie.size(), static_cast<std::size_t>(kRouteCount));

    const std::uint32_t probe = (static_cast<std::uint32_t>(10) << 24) |
                                 (static_cast<std::uint32_t>(2500 >> 8) << 8) |
                                 static_cast<std::uint32_t>(2500 & 0xFF);
    const auto match = trie.longestPrefixMatch(probe);
    REQUIRE(match.has_value());
}
