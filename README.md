# **General Directed Weighted Graph**

**Type:** C++ Library (Header-first) · **Tech:** C++20, STL · **Status:** Completed

## **Overview:**
Value-semantics directed multigraph `gdwg::graph<N,E>` with unique nodes and no duplicate edges, supporting both unweighted and weighted edges with strict ordering and full iterator/range support.

## **Highlights**

* **Edges:** Abstract `edge<N,E>` API (`print_edge()`, `is_weighted()`, `get_weight()`, `get_nodes()`, `operator==`); concrete `unweighted_edge` and `weighted_edge` with ordering by `(src,dst,weight)` and unweighted before weighted.
* **Core Ops:** `insert_node`, `insert_edge(src,dst,opt<weight>)`, `replace_node`, `merge_replace_node` (redirects and de-dupes), `erase_node`, `erase_edge` (by key or iterator[s]), `clear() noexcept`.
* **Queries:** `is_node`, `empty`, `is_connected`, `nodes()` (sorted), `edges(src,dst)` (unweighted first, then weight-asc), `find`, `connections(src)`; complexity bounds respected.
* **Iteration:** Bidirectional edge iterator over `{from,to,optional<weight>}`; `begin/end` const; nodes without edges skipped.
* **Semantics:** Owns copies; moved-from becomes empty; equality compares node and edge sets; `operator<<` prints nodes with outgoing edges; `const`/`noexcept` where sensible.
* **I/O & Conversions:** Clean stream output; no accidental allocations; initialisers from ranges and lists; full copy/move ctor/assign.

## **Why it's interesting:** 
Demonstrates clean value semantics, disciplined ordering and invariants, efficient associative data use, and a sharp API surface suitable for teaching graph ADTs and testing.
