# Architecture

## Component Overview

| Component | Responsibility | Depends on |
|---|---|---|
| `Trie` | Stores routes, answers exact-match and longest-prefix-match queries | (none) |
| `Graph` | Stores network topology as a weighted adjacency list | (none) |
| `UnionFind` | Disjoint-set structure for cycle detection | (none) |
| `dijkstra` / `bellmanFord` | Compute shortest paths over a `Graph` | `Graph` |
| `kruskalMST` | Compute minimum spanning tree over a `Graph` | `Graph`, `UnionFind` |
| `NetworkLoader` | Parses a network file into a `Graph` + `Trie` | `Graph`, `Trie` |
| `main.cpp` (CLI) | argv parsing, dispatch, formatted output | everything above |

Each component is independently unit-tested (see `tests/`), and none of
the algorithm implementations depend on the CLI — `main.cpp` is the only
file that touches `iostream`/argv.

## Data Flow: `netroute shortest-path <file> <A> <B>`

```
  argv
   |
   v
main() -- parses subcommand + args
   |
   v
loadNetworkFile(file) --------> Network { Graph topology, Trie routingTable }
   |
   v
topology.indexOf("A"), topology.indexOf("B")  -- resolve labels to indices
   |
   v
dijkstra(topology, sourceIndex) --------> ShortestPathResult { distance[], predecessor[] }
   |
   v
result.pathTo(targetIndex) -------------> reconstructed path via predecessor chain
   |
   v
formatted stdout output
```

## Data Flow: `netroute route <file> <ip>`

```
  argv
   |
   v
loadNetworkFile(file) --------> Network { Graph topology, Trie routingTable }
   |
   v
Trie::parseIPv4(ip) -----------> uint32_t address
   |
   v
routingTable.longestPrefixMatch(address)
   |
   v
walks the trie bit-by-bit from the MSB, remembering the deepest
node marked as a route along the path (this IS longest-prefix-match)
   |
   v
formatted stdout output: matched prefix, next hop, metric
```

## Why a Trie for the Routing Table

A naive routing table implementation stores routes in a list and, for
each incoming packet, scans every entry checking whether the
destination falls inside that route's prefix, keeping the most specific
(longest) match. That's O(R) per lookup where R is the number of routes
— fine for a handful of routes, but a real routing table can have
hundreds of thousands of entries.

A trie turns this into a bounded-depth walk: each bit of the 32-bit
address selects a left/right child, and the deepest node marked as a
route along that walk is the longest prefix match. Lookup cost becomes
O(32) — a small constant — independent of how many routes are stored.
This is conceptually the same reason real hardware routers use tries
(or content-addressable memory, which is a hardware-parallel version of
the same idea) instead of scanning a route list.

## Why Union-Find for Kruskal

Kruskal's algorithm needs to answer one question repeatedly while
processing edges in increasing weight order: *"are these two nodes
already connected by edges I've already picked?"* Naively, checking
connectivity would require a graph traversal (BFS/DFS) per edge — O(V +
E) each time, O(E · (V + E)) total.

Union-Find answers the same question in near-constant amortized time
via two techniques working together:

- **Path compression** — every `find()` call flattens the tree so future
  lookups on the same nodes are faster.
- **Union by rank** — when merging two sets, the shorter tree is
  attached under the taller one, keeping trees shallow.

Together these give O(α(V)) amortized per operation, where α is the
inverse Ackermann function — for any input size that could physically
exist, α(V) ≤ 4, so this is effectively constant time.
