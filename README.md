# General Directed Weighted Graph

* Implements `gdwg::graph<N,E>`: a value-semantics, directed multigraph with both unweighted and weighted edges.
* Owns copies of node and edge data; no redundant duplicates; moved-from graphs become empty.
* Enforces unique nodes; allows reflexive edges; forbids duplicate (src,dst,weight).
* Orders edges by (src, dst, then weight asc); unweighted precede weighted for same (src,dst).
* Provides edge hierarchy: abstract `edge<N,E>` with `print_edge()`, `is_weighted()`, `get_weight()`, `get_nodes()`, `operator==`, and virtual dtor.
* `weighted_edge` stores src/dst/weight; `unweighted_edge` stores src/dst; both implement base API.
* Constructors: default; from `initializer_list`/iterator range; copy/move; copy/move assign.
* Modifiers:

  * `insert_node(v)`: add node if absent.
  * `insert_edge(src,dst,opt<weight>)`: add un/weighted edge; error if nodes missing.
  * `replace_node(old,new)`: rename node if `new` absent; error if `old` missing.
  * `merge_replace_node(old,new)`: redirect all edges from/to `old` to `new`, removing duplicates; errors if either missing.
  * `erase_node(v)`: delete node and all incident edges.
  * `erase_edge(src,dst,opt<weight>)`: delete specific edge; error if nodes missing.
  * `erase_edge(it)` / `erase_edge(it, it2)`: erase by iterator(s); return post-erase iterator.
  * `clear() noexcept`: remove all nodes/edges.
* Accessors:

  * `is_node(v)`: membership (`O(log n)`).
  * `empty()`: no nodes.
  * `is_connected(src,dst)`: any edge exists; errors if nodes missing.
  * `nodes()`: vector copy of all nodes, ascending.
  * `edges(src,dst)`: vector of copies of edges from src→dst, unweighted first, then weighted asc; errors if nodes missing.
  * `find(src,dst,opt<weight>)`: iterator to matching edge or `end()`.
  * `connections(src)`: sorted unique destinations from src; errors if src missing.
* Iteration: bidirectional `graph::iterator` over edges only (nodes without edges are skipped); deref yields `{from,to,optional<weight>}`.
* Range: `begin()/end()` (const) define iterable edge list; supports `++/--`, `==`.
* Equality: `operator==` true iff graphs have identical node sets and edge sets (including weights/unweighted).
* Output: `operator<<` prints each node and its outgoing edges using the specified format; unweighted first, then weighted asc.
* Complexity notes respected (e.g., `edges/find/erase` with `log n + e` style bounds).
* Applies `const`/`noexcept` where appropriate; no extra public members beyond spec (friend non-member ops allowed).
