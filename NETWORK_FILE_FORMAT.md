# Network File Format

NetRoute Engine reads a simple, line-oriented text format describing a
network's topology (`LINK`) and routing table (`ROUTE`). See
`examples/network.txt` for a complete working example.

## Directives

### `LINK <nodeA> <nodeB> <weight>`

Declares a bidirectional link between `nodeA` and `nodeB` with the given
integer `weight` (link cost). Node names are created automatically the
first time they're referenced — there is no separate node-declaration
step.

```
LINK RouterA RouterB 4
```

Weights may be negative (Bellman-Ford tolerates this; Dijkstra will
reject the graph with a clear error if you try to run it against a
negative-weight edge).

### `ROUTE <prefix>/<prefixLength> <nextHop> [metric]`

Declares a routing table entry. `<prefix>/<prefixLength>` is standard
CIDR notation. `<nextHop>` is a free-form label (does not need to match
a `LINK` node name, though typically it will). `[metric]` is optional
and defaults to `1`.

```
ROUTE 192.168.1.0/24 RouterD 1
ROUTE 0.0.0.0/0 RouterA          # metric defaults to 1
```

## Comments and Blank Lines

Anything from `#` to the end of a line is treated as a comment and
ignored. Blank lines are ignored.

```
# This entire line is a comment
LINK A B 1   # trailing comments are also supported
```

## Error Handling

Malformed lines raise `NetworkFileError`, which includes the offending
line number:

| Malformed input | Error |
|---|---|
| `TELEPORT A B` | unknown directive |
| `LINK OnlyOneToken` | wrong number of arguments |
| `LINK A B not-a-number` | invalid weight |
| `ROUTE 10.0.0.0 RouterA` | missing `/` — not valid CIDR |
| `ROUTE 10.0.0.0/99 RouterA` | prefix length out of `[0, 32]` range |
| `ROUTE 999.1.1.1/8 RouterA` | invalid IPv4 octet |

A missing file raises a plain `std::runtime_error` (no line number,
since the file was never opened).
