# Project Structure

```
netroute-engine/
│
├── include/netroute/          Public library headers
│   ├── trie.hpp                Binary trie: LPM routing table
│   ├── graph.hpp                Adjacency-list weighted graph
│   ├── union_find.hpp            Disjoint Set Union (Kruskal helper)
│   ├── shortest_path.hpp          Dijkstra + Bellman-Ford
│   ├── kruskal.hpp                 Minimum Spanning Tree
│   └── network_loader.hpp          LINK/ROUTE file format parser
│
├── src/                        Implementations + CLI
│   ├── trie.cpp
│   ├── graph.cpp
│   ├── shortest_path.cpp
│   ├── kruskal.cpp
│   ├── network_loader.cpp
│   └── main.cpp                    CLI entry point (thin argv wrapper)
│
├── tests/                      Unit tests (dependency-free harness)
│   ├── test_framework.hpp          TEST_CASE / REQUIRE / REQUIRE_THROWS_AS
│   ├── test_trie.cpp
│   ├── test_shortest_path.cpp
│   ├── test_kruskal.cpp
│   ├── test_union_find.cpp
│   ├── test_network_loader.cpp
│   └── test_main.cpp                Aggregates and runs all TEST_CASEs
│
├── examples/
│   └── network.txt                  Sample topology used throughout the README
│
├── docs/
│   ├── ARCHITECTURE.md              Component responsibilities + data flow
│   ├── NETWORK_FILE_FORMAT.md        LINK/ROUTE syntax reference
│   └── DEVELOPER_GUIDE.md            How to extend the engine
│
├── assets/                     Space reserved for screenshots/recordings
│
├── build/                      Generated at build time (git-ignored)
│
├── CMakeLists.txt              CMake build definition
├── Makefile                    Primary build path (no cmake dependency)
├── README.md
├── LICENSE                     MIT
├── CONTRIBUTING.md
├── CHANGELOG.md
├── CODE_OF_CONDUCT.md
├── PROJECT_STRUCTURE.md        This file
└── .gitignore
```

## Layering Rules

1. `src/main.cpp` may depend on anything in `include/netroute/`, but
   nothing in `include/netroute/` may depend on `main.cpp`. This keeps
   every algorithm testable without going through the CLI.
2. `network_loader` depends on `graph` and `trie`, but `graph` and
   `trie` do not depend on `network_loader` — the file format is a
   concern layered on top of the core data structures, not baked into
   them.
3. `kruskal` and `shortest_path` both depend on `graph`, but not on each
   other.
4. `union_find` has no dependencies within the project — it is a
   general-purpose utility only used internally by `kruskal.cpp`.
