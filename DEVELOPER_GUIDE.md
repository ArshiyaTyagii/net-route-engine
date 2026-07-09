# Developer Guide

## Building for Development

```bash
make clean && make test
```

Run this before and after any change. A change that breaks any of the
37 existing tests should not be merged without either fixing the
regression or updating the test if the expected behavior intentionally
changed (and explaining why in the PR).

## Adding a New CLI Subcommand

1. Implement the underlying logic as a free function or class in
   `include/netroute/` + `src/`, fully independent of argv/iostream.
2. Add unit tests for that logic in `tests/`.
3. Only after the library-level tests pass, add a `cmdYourCommand(...)`
   function in `src/main.cpp` and wire it into the `if (command == ...)`
   dispatch chain.
4. Update `printUsage()` and the README's Usage section.

This ordering (library first, CLI last) is deliberate — it's the
difference between a project where the algorithms are testable and one
where you can only verify behavior by eyeballing CLI output.

## Extending the Trie for IPv6

The current `Trie` uses a fixed 32-bit key space matching IPv4. To
support IPv6:

- Change the address representation from `uint32_t` to a 128-bit type
  (e.g. `std::array<std::uint8_t, 16>` or two `uint64_t` halves).
- The bit-walk logic in `insert`/`search`/`longestPrefixMatch` stays
  structurally the same — only the depth bound changes from 32 to 128.
- `parseIPv4`/`formatIPv4` would need IPv6-equivalent counterparts
  (colon-hex parsing, `::` compression handling).

## Extending Graph for Directed-Only MST Input

`Kruskal::allEdgesUnique()` currently assumes an undirected graph (it
reports each `(from, to)` pair once when `from < to`). If you need MST
support over a graph built with `directed=true` edges that lack a
reverse counterpart, you'll need a variant of `allEdgesUnique()` that
does not rely on the reverse edge being present — Kruskal's algorithm
itself is only meaningfully defined over undirected graphs, so this is
a data-representation concern, not an algorithmic one.

## Debugging a Failing Test

The test harness prints a `[ FAIL ]` line with the file and line number
of the failed `REQUIRE`/`REQUIRE_EQ`/`REQUIRE_THROWS_AS`:

```
[ FAIL ] trie_longest_prefix_match_prefers_most_specific -- REQUIRE_EQ(match->prefixLength, 24) failed at tests/test_trie.cpp:52 -- lhs=16 rhs=24
```

Since every test is a plain function registered via a static
`Registrar`, you can also build and debug a single test file's object
directly with `gdb`/`lldb` against `build/netroute_tests` — no special
test-runner invocation syntax is needed.
