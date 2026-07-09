# NetRoute Engine

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)
![Tests](https://img.shields.io/badge/tests-37%20passing-brightgreen.svg)
![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)
![Warnings](https://img.shields.io/badge/warnings-zero%20(--Wall%20--Wextra%20--Wpedantic)-brightgreen.svg)

A command-line network routing simulator implementing the core data
structures and algorithms behind real router forwarding planes and
network topology design: a **trie-based routing table** for
longest-prefix-match, **Dijkstra's** and **Bellman-Ford's** shortest-path
algorithms, and **Kruskal's minimum spanning tree** for lowest-cost
backbone design.

## Table of Contents

- [Motivation](#motivation)
- [Features](#features)
- [Architecture](#architecture)
- [Folder Structure](#folder-structure)
- [Algorithms Implemented](#algorithms-implemented)
- [Complexity](#complexity)
- [Design Decisions](#design-decisions)
- [Installation](#installation)
- [Build Instructions](#build-instructions)
- [Usage](#usage)
- [Testing](#testing)
- [Screenshots](#screenshots)
- [Future Improvements](#future-improvements)
- [Learning Outcomes](#learning-outcomes)
- [License](#license)

## Motivation

Most textbook implementations of tries, Dijkstra, Bellman-Ford, and
Kruskal exist as isolated 40-line snippets solving a single LeetCode-style
problem. NetRoute Engine instead ties all four together around one
coherent domain — network routing — the same way a real router's control
plane (path selection, topology, forwarding table) and data plane
(longest-prefix-match lookup) fit together. The goal was to practice
writing algorithms as part of a *system* with a real API, real file
format, real CLI, and a real test suite, not as a standalone function.

## Features

- **Trie-based routing table** supporting `insert`, `remove`, exact
  `search`, and `longestPrefixMatch` (LPM) — the operation every IP
  router performs on every packet.
- **Dijkstra's algorithm** (binary-heap priority queue) for shortest
  paths over non-negative-weight links, with explicit rejection of
  negative weights rather than silently wrong output.
- **Bellman-Ford's algorithm** for shortest paths that tolerate negative
  edge weights, including negative-cycle detection.
- **Kruskal's Minimum Spanning Tree** (Union-Find with union-by-rank and
  path compression) for lowest-cost network backbone design, including
  correct handling of disconnected topologies.
- A small, readable **network description file format** (`LINK` /
  `ROUTE` directives) with line-numbered error messages on malformed
  input.
- A **CLI** (`netroute`) exposing every algorithm as a subcommand.
- **37 unit tests** covering normal cases, edge cases (host routes,
  default routes, disconnected graphs, negative cycles), and malformed
  input — running via a self-contained, dependency-free test harness
  (`tests/test_framework.hpp`).
- Compiles **clean with zero warnings** under `-Wall -Wextra -Wpedantic`.

## Architecture

```
                         +---------------------+
                         |    CLI (main.cpp)   |
                         |  argv parsing only   |
                         +----------+-----------+
                                    |
             +----------------------+----------------------+
             |                      |                       |
     +-------v-------+     +--------v--------+     +--------v--------+
     | NetworkLoader  |     |  ShortestPath   |     |     Kruskal     |
     | (file parsing) |     | Dijkstra / BF   |     |       MST       |
     +-------+--------+     +--------+--------+     +--------+--------+
             |                       |                       |
     +-------v--------+     +--------v--------+              |
     |      Trie       |     |      Graph      |<-------------+
     | (routing table) |     | (adjacency list)|
     +-----------------+     +--------+--------+
                                       |
                              +--------v--------+
                              |    UnionFind     |
                              | (Kruskal helper) |
                              +-------------------+
```

The CLI is a thin argv-parsing layer only — every algorithm and data
structure lives in the library (`include/netroute/*.hpp`,
`src/*.cpp`) and is independently unit-tested without going through
`main()`.

## Folder Structure

```
netroute-engine/
├── include/netroute/     Public headers (one per component)
│   ├── trie.hpp
│   ├── graph.hpp
│   ├── union_find.hpp
│   ├── shortest_path.hpp
│   ├── kruskal.hpp
│   └── network_loader.hpp
├── src/                  Implementations + CLI entry point
│   ├── trie.cpp
│   ├── graph.cpp
│   ├── shortest_path.cpp
│   ├── kruskal.cpp
│   ├── network_loader.cpp
│   └── main.cpp
├── tests/                 Dependency-free unit tests
│   ├── test_framework.hpp
│   ├── test_trie.cpp
│   ├── test_shortest_path.cpp
│   ├── test_kruskal.cpp
│   ├── test_union_find.cpp
│   ├── test_network_loader.cpp
│   └── test_main.cpp
├── examples/
│   └── network.txt         Sample network file used in the walkthrough below
├── docs/                    Architecture notes, complexity tables, diagrams
├── assets/                   Space for screenshots / recorded terminal sessions
├── CMakeLists.txt
├── Makefile
└── README.md
```

## Algorithms Implemented

| Algorithm | File | Purpose |
|---|---|---|
| Trie (binary, prefix-indexed) | `trie.hpp` / `trie.cpp` | Longest-prefix-match routing table lookups |
| Dijkstra | `shortest_path.hpp` / `shortest_path.cpp` | Shortest path, non-negative weights |
| Bellman-Ford | `shortest_path.hpp` / `shortest_path.cpp` | Shortest path with negative weights + cycle detection |
| Kruskal's MST | `kruskal.hpp` / `kruskal.cpp` | Minimum-cost spanning backbone |
| Union-Find (path compression + union-by-rank) | `union_find.hpp` | Kruskal cycle detection |

## Complexity

**Time complexity**

| Operation | Complexity | Notes |
|---|---|---|
| `Trie::insert` | O(32) = O(1) | Bounded by IPv4 address width |
| `Trie::search` (exact) | O(32) = O(1) | |
| `Trie::longestPrefixMatch` | O(32) = O(1) | |
| `Trie::remove` | O(32) = O(1) | Includes dead-branch pruning |
| `dijkstra` | O((V + E) log V) | Binary-heap priority queue |
| `bellmanFord` | O(V · E) | Early-exits on convergence |
| `kruskalMST` | O(E log E) | Dominated by the edge sort |
| `UnionFind::find` / `unite` | O(α(V)) amortized | Inverse Ackermann — effectively constant |

**Space complexity**

| Structure | Complexity | Notes |
|---|---|---|
| Trie | O(R · 32) | R = number of routes, 32 = max IPv4 prefix depth |
| Graph (adjacency list) | O(V + E) | Sparse representation |
| Dijkstra / Bellman-Ford working set | O(V) | Distance + predecessor arrays |
| Union-Find | O(V) | Parent + rank arrays |

## Design Decisions

- **Adjacency list over adjacency matrix.** Network topologies are
  sparse (a router peers with a handful of neighbors, not every other
  router in the network), so adjacency lists give O(V + E) space instead
  of O(V²) and O(deg(v)) neighbor iteration instead of O(V).
- **Trie over a naive prefix scan.** A linear scan of all routes to find
  the longest match is O(R) per lookup; the trie makes every lookup
  O(32), independent of table size — the same reason real routers use
  tries (or hardware TCAMs) instead of scanning a route list.
- **Dijkstra rejects negative weights explicitly** rather than silently
  producing an incorrect answer, since Dijkstra's correctness proof
  assumes non-negative weights. Bellman-Ford is provided as the
  general-purpose fallback and additionally detects negative cycles.
- **RAII throughout.** The trie owns its nodes via `std::unique_ptr`; no
  raw `new`/`delete` appears anywhere in the codebase, so the tree
  destroys itself correctly with no manual cleanup and no leaks.
- **Custom lightweight test harness instead of a testing framework
  dependency.** `tests/test_framework.hpp` is ~90 lines and gives
  `TEST_CASE` / `REQUIRE` / `REQUIRE_THROWS_AS` macros. This keeps `make
  test` working offline with nothing but a C++17 compiler — no package
  manager, no network access, no version pinning to worry about.

## Installation

Requires a C++17 compiler (GCC ≥ 9, Clang ≥ 10, or MSVC ≥ 19.20) and
`make`. `cmake` is optional — a `Makefile` is provided as the primary
build path and was used to produce every example in this README.

```bash
git clone https://github.com/<your-username>/netroute-engine.git
cd netroute-engine
```

## Build Instructions

**Using make (recommended, no dependencies beyond a C++17 compiler):**

```bash
make cli    # builds build/netroute
make test   # builds and runs build/netroute_tests
make clean  # removes build/ and any leftover test temp files
```

**Using CMake:**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
```

Both paths compile with `-Wall -Wextra -Wpedantic` and produce zero
warnings.

## Usage

All examples below use [`examples/network.txt`](examples/network.txt):

```
LINK RouterA RouterB 4
LINK RouterA RouterC 1
LINK RouterC RouterB 1
LINK RouterB RouterD 3
LINK RouterC RouterD 6
LINK RouterD RouterE 2

ROUTE 0.0.0.0/0        RouterA   10
ROUTE 10.0.0.0/8       RouterB   1
ROUTE 10.1.0.0/16      RouterC   1
ROUTE 192.168.1.0/24   RouterD   1
ROUTE 192.168.1.128/25 RouterE   1
```

### `load` — parse and validate a network file

```bash
$ netroute load examples/network.txt
Loaded network from examples/network.txt
  Nodes:  5
  Links:  6
  Routes: 5
```

### `shortest-path` — Dijkstra

```bash
$ netroute shortest-path examples/network.txt RouterA RouterD
Dijkstra shortest path RouterA -> RouterD
  Cost: 5
  Path: RouterA -> RouterC -> RouterB -> RouterD
```

### `shortest-path-bf` — Bellman-Ford (tolerates negative weights)

```bash
$ netroute shortest-path-bf examples/network.txt RouterA RouterE
Bellman-Ford shortest path RouterA -> RouterE
  Cost: 7
  Path: RouterA -> RouterC -> RouterB -> RouterD -> RouterE
```

### `mst` — Kruskal's Minimum Spanning Tree

```bash
$ netroute mst examples/network.txt
Kruskal Minimum Spanning Tree
  RouterA -- RouterC  (weight 1)
  RouterB -- RouterC  (weight 1)
  RouterD -- RouterE  (weight 2)
  RouterB -- RouterD  (weight 3)
Total weight: 7
```

### `route` — Longest-Prefix-Match lookup

```bash
$ netroute route examples/network.txt 192.168.1.25
192.168.1.25 -> 192.168.1.0/24 via RouterD (metric 1)

$ netroute route examples/network.txt 10.1.5.5
10.1.5.5 -> 10.1.0.0/16 via RouterC (metric 1)

$ netroute route examples/network.txt 8.8.8.8
8.8.8.8 -> 0.0.0.0/0 via RouterA (metric 10)
```

Note how `192.168.1.25` matches the more specific `/24` over the `/0`
default route, and an address with no specific match falls back to the
default route — this is longest-prefix-match behaving exactly as it does
on a real router.

### `print-routing-table` — dump the loaded routes

```bash
$ netroute print-routing-table examples/network.txt
Routing Table (examples/network.txt)
  ROUTE 0.0.0.0/0        RouterA   10
  ROUTE 10.0.0.0/8       RouterB   1
  ROUTE 10.1.0.0/16      RouterC   1
  ROUTE 192.168.1.0/24   RouterD   1
  ROUTE 192.168.1.128/25 RouterE   1
```

### Error handling

```bash
$ netroute shortest-path examples/network.txt RouterA RouterZ
error: unknown node 'RouterZ'

$ netroute route examples/network.txt 999.1.1.1
error: octet out of range in '999.1.1.1'
```

## Testing

```bash
$ make test
...
37 passed, 0 failed, 37 total
```

Tests cover, per component:

- **Trie:** exact search, LPM specificity ordering, default route
  (`/0`), host routes (`/32`), overwrite-on-duplicate-insert, remove +
  branch pruning, malformed IPv4 parsing, a 5,000-route bulk
  insert/lookup sanity check.
- **Dijkstra / Bellman-Ford:** shortest-path correctness, negative-weight
  rejection (Dijkstra), negative-weight tolerance and negative-cycle
  detection (Bellman-Ford), unreachable-node handling.
- **Kruskal / Union-Find:** MST correctness, disconnected-graph
  handling, parallel-edge handling, transitive connectivity.
- **Network loader:** valid files, comments/blank lines, every
  malformed-input case (unknown directive, wrong arity, non-numeric
  weight, missing CIDR slash, out-of-range prefix length, missing file).

## Screenshots

*(Reserved for a recorded terminal session / asciinema cast — add to
`assets/` and link it here.)*

## Future Improvements

- IPv6 support (128-bit trie keys instead of 32-bit)
- A `benchmark/` target with real timing measurements against
  randomly generated large topologies (the current performance test is
  a correctness sanity check, not a benchmark)
- JSON output mode for CLI commands to make the tool scriptable
- A* search as an additional shortest-path option for geographically
  weighted topologies
- Persistent routing table snapshots (save/load trie state to disk)

## Learning Outcomes

Building this project involved:

- Implementing a trie with real removal + branch pruning (not just
  insert/search, which is the version most tutorials stop at)
- Distinguishing when to use Dijkstra vs. Bellman-Ford, and why
  Dijkstra's non-negative-weight assumption is a correctness
  requirement, not a convenience
- Implementing Union-Find with both path compression and union-by-rank,
  and understanding why both together give the near-constant amortized
  bound
- Designing a small library with a clean CLI/library boundary, so every
  algorithm is unit-testable independent of argv parsing
- Writing a dependency-free test harness, since C++ (unlike JS/Python)
  has no test runner built in

## License

Released under the [MIT License](LICENSE).
