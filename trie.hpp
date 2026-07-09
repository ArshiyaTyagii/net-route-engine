// trie.hpp
// Binary trie over IPv4 prefixes supporting insert, remove, exact search,
// and longest-prefix-match (LPM) lookup -- the core data structure behind
// hardware and software routing tables.
//
// Design notes
// ------------
// * Addresses are represented as 32-bit unsigned integers (network byte
//   order is irrelevant here; we only care about bit ordering for the
//   trie walk).
// * Each node owns its children via std::unique_ptr, so the trie cleans
//   itself up automatically (RAII) -- no manual delete, no leaks.
// * A node is a "prefix node" (isEndOfPrefix_ == true) when a route was
//   explicitly inserted terminating at that node. Intermediate nodes may
//   exist purely as path nodes with no attached route.
// * Complexity: insert/remove/search are O(prefixLength) bit operations,
//   independent of how many routes are stored -- this is why tries beat
//   naive linear prefix scans for large routing tables.

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace netroute {

// A single routing entry attached to a trie node.
struct RouteEntry {
    std::uint32_t prefix{};      // Network address (host byte order)
    std::uint8_t prefixLength{}; // CIDR prefix length, 0-32
    std::string nextHop;         // Symbolic next-hop / interface name
    int metric{1};                // Route cost/metric, used as edge weight

    RouteEntry() = default;
    RouteEntry(std::uint32_t prefix_, std::uint8_t prefixLength_,
               std::string nextHop_, int metric_ = 1)
        : prefix(prefix_), prefixLength(prefixLength_),
          nextHop(std::move(nextHop_)), metric(metric_) {}
};

// Thrown when an operation receives a structurally invalid prefix length
// or a malformed dotted-quad address string.
class InvalidPrefixError : public std::invalid_argument {
public:
    explicit InvalidPrefixError(const std::string& what)
        : std::invalid_argument(what) {}
};

class Trie {
public:
    Trie() : root_(std::make_unique<Node>()) {}

    // Non-copyable (owns a unique_ptr tree); movable.
    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;
    Trie(Trie&&) noexcept = default;
    Trie& operator=(Trie&&) noexcept = default;

    // Inserts (or overwrites) a route for the given prefix/prefixLength.
    // Throws InvalidPrefixError if prefixLength > 32.
    void insert(const RouteEntry& entry);

    // Removes the route exactly matching prefix/prefixLength, if present.
    // Returns true if a route was removed, false if no exact match existed.
    // Prunes now-empty intermediate nodes to keep the trie compact.
    bool remove(std::uint32_t prefix, std::uint8_t prefixLength);

    // Exact-match search: returns the route only if a route was inserted
    // with precisely this prefix/prefixLength pair.
    [[nodiscard]] std::optional<RouteEntry> search(std::uint32_t prefix,
                                                     std::uint8_t prefixLength) const;

    // Longest-prefix-match lookup for a destination address. Walks the
    // trie bit-by-bit from the MSB, remembering the deepest node marked
    // as a route along the path. This is the operation real routers use
    // to decide where to forward a packet.
    [[nodiscard]] std::optional<RouteEntry> longestPrefixMatch(std::uint32_t address) const;

    // Number of distinct routes currently stored.
    [[nodiscard]] std::size_t size() const noexcept { return routeCount_; }
    [[nodiscard]] bool empty() const noexcept { return routeCount_ == 0; }

    // Parses a dotted-quad IPv4 string ("192.168.1.0") into a uint32_t.
    // Throws InvalidPrefixError on malformed input.
    static std::uint32_t parseIPv4(const std::string& dottedQuad);

    // Formats a uint32_t address back into dotted-quad notation.
    static std::string formatIPv4(std::uint32_t address);

private:
    struct Node {
        std::array<std::unique_ptr<Node>, 2> children{};
        std::optional<RouteEntry> route;

        [[nodiscard]] bool isLeaf() const noexcept {
            return !children[0] && !children[1];
        }
    };

    std::unique_ptr<Node> root_;
    std::size_t routeCount_{0};

    static void validatePrefixLength(std::uint8_t prefixLength);

    // Recursive helper for remove(); returns true if `node` should be
    // pruned by its parent after the call (i.e. it is now a childless,
    // routeless node).
    static bool removeRecursive(Node* node, std::uint32_t prefix,
                                 std::uint8_t prefixLength, std::uint8_t depth,
                                 bool& removed);
};

} // namespace netroute
