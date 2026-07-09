// network_loader.hpp
// Parses a simple, human-editable network description file into a Graph
// and a Trie-backed routing table. See docs/NETWORK_FILE_FORMAT.md and
// examples/network.txt for the file format.
//
// File format
// -----------
//   # comments start with '#' and are ignored
//   LINK <nodeA> <nodeB> <weight>
//   ROUTE <prefix>/<prefixLength> <nextHop> [metric]
//
// Example:
//   LINK RouterA RouterB 4
//   ROUTE 192.168.1.0/24 RouterA 1

#pragma once

#include <string>

#include "netroute/graph.hpp"
#include "netroute/trie.hpp"

namespace netroute {

class NetworkFileError : public std::runtime_error {
public:
    NetworkFileError(const std::string& what, int lineNumber)
        : std::runtime_error("line " + std::to_string(lineNumber) + ": " + what) {}
};

struct Network {
    Graph topology;
    Trie routingTable;
};

// Parses the file at `path` into a Network. Throws NetworkFileError on
// malformed lines (with the offending line number) and std::runtime_error
// if the file cannot be opened.
Network loadNetworkFile(const std::string& path);

} // namespace netroute
