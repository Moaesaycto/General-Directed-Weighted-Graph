#include "gdwg_graph.h"

#include <catch2/catch.hpp>
#include <iostream>

TEST_CASE("basic test") {
	// These are commented out right now
	//  because without your implementation
	//  it will not compile. Uncomment them
	//  once you've done the work
	auto g = gdwg::graph<int, std::string>{};
	auto n = 5;
	g.insert_node(n);
	CHECK(g.is_node(n));
}

TEST_CASE("Node Insertion") {
	using graph = gdwg::graph<int, int>;
	auto g = graph{};
	CHECK(g.insert_node(1) == true);
	CHECK(g.insert_node(1) == false);
	CHECK(g.is_node(1) == true);
	CHECK(g.is_node(2) == false);
}

TEST_CASE("Edge Insertion") {
	using graph = gdwg::graph<int, int>;
	auto g = graph{};
	g.insert_node(1);
	g.insert_node(2);
	CHECK(g.insert_edge(1, 2, 10) == true);
	CHECK(g.insert_edge(1, 2, 10) == false);
	CHECK(g.insert_edge(1, 2) == true);
	CHECK_THROWS_WITH(g.insert_edge(1, 3, 5),
	                  "Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node does not exist");
}

TEST_CASE("Replace Node") {
	using graph = gdwg::graph<int, int>;
	auto g = graph{};
	g.insert_node(1);
	g.insert_node(2);
	g.insert_edge(1, 2, 10);
	CHECK(g.replace_node(1, 3) == true);
	CHECK(g.is_node(1) == false);
	CHECK(g.is_node(3) == true);
	CHECK(g.replace_node(2, 3) == false);
	CHECK_THROWS_WITH(g.replace_node(4, 5), "Cannot call gdwg::graph<N, E>::replace_node on a node that doesn't exist");
}

TEST_CASE("Merge Replace Node") {
	using graph = gdwg::graph<int, int>;
	auto g = graph{};
	g.insert_node(1);
	g.insert_node(2);
	g.insert_node(3);
	g.insert_edge(1, 2, 10);
	g.insert_edge(1, 3, 15);
	g.merge_replace_node(1, 2);
	CHECK(g.is_node(1) == false);
	CHECK(g.is_node(2) == true);
	CHECK(g.is_connected(2, 3) == true);
	CHECK_THROWS_WITH(g.merge_replace_node(1, 4),
	                  "Cannot call gdwg::graph<N, E>::merge_replace_node on old or new data if they don't exist in the "
	                  "graph");
}

TEST_CASE("Erase Node") {
	using graph = gdwg::graph<int, int>;
	auto g = graph{};
	g.insert_node(1);
	g.insert_node(2);
	g.insert_edge(1, 2, 10);

	CHECK(g.erase_node(1) == true);
	CHECK(g.is_node(1) == false);
	CHECK(g.is_node(2) == true); // Ensure node 2 still exists
	CHECK_THROWS_WITH(g.is_connected(1, 2),
	                  "Cannot call gdwg::graph<N, E>::is_connected if src or dst node don't exist in the graph");
	CHECK(g.erase_node(3) == false);
}

TEST_CASE("Erase Edge") {
	using graph = gdwg::graph<int, int>;
	auto g = graph{};
	g.insert_node(1);
	g.insert_node(2);
	g.insert_edge(1, 2, 10);
	CHECK(g.erase_edge(1, 2, 10) == true);
	CHECK(g.is_connected(1, 2) == false);
	CHECK(g.erase_edge(1, 2, 5) == false);
	CHECK_THROWS_WITH(g.erase_edge(1, 3, 5),
	                  "Cannot call gdwg::graph<N, E>::erase_edge on src or dst if they don't exist in the graph");
}

TEST_CASE("Clear Graph") {
	using graph = gdwg::graph<int, int>;
	auto g = graph{};
	g.insert_node(1);
	g.insert_node(2);
	g.insert_edge(1, 2, 10);
	g.clear();
	CHECK(g.empty() == true);
}

TEST_CASE("Equality Operator") {
	using graph = gdwg::graph<int, int>;
	auto g1 = graph{};
	auto g2 = graph{};
	g1.insert_node(1);
	g1.insert_node(2);
	g1.insert_edge(1, 2, 10);
	g2.insert_node(1);
	g2.insert_node(2);
	g2.insert_edge(1, 2, 10);
	CHECK(g1 == g2);
	g2.insert_edge(2, 1, 5);
	CHECK_FALSE(g1 == g2);
}

TEST_CASE("Output Operator") {
	using graph = gdwg::graph<int, int>;

	auto const v = std::vector<std::tuple<int, int, std::optional<int>>>{
	    {4, 1, -4},
	    {3, 2, 2},
	    {2, 4, std::nullopt},
	    {2, 4, 2},
	    {2, 1, 1},
	    {4, 1, std::nullopt},
	    {6, 2, 5},
	    {6, 3, 10},
	    {1, 5, -1},
	    {3, 6, -8},
	    {4, 5, 3},
	    {5, 2, std::nullopt},
	};

	auto g = graph{};
	for (const auto& [from, to, weight] : v) {
		g.insert_node(from);
		g.insert_node(to);
		if (weight.has_value()) {
			g.insert_edge(from, to, weight.value());
		}
		else {
			g.insert_edge(from, to);
		}
	}
	g.insert_node(64);

	auto out = std::ostringstream{};
	out << g;
	auto const expected_output = std::string_view(R"(
1 (
  1 -> 5 | W | -1
)
2 (
  2 -> 4 | U
  2 -> 1 | W | 1
  2 -> 4 | W | 2
)
3 (
  3 -> 2 | W | 2
  3 -> 6 | W | -8
)
4 (
  4 -> 1 | U
  4 -> 1 | W | -4
  4 -> 5 | W | 3
)
5 (
  5 -> 2 | U
)
6 (
  6 -> 2 | W | 5
  6 -> 3 | W | 10
)
64 (
)
)");
	CHECK(out.str() == expected_output);
}

TEST_CASE("Iterator Test") {
	using graph = gdwg::graph<int, int>;
	auto g = graph{};
	g.insert_node(1);
	g.insert_node(2);
	g.insert_edge(1, 2, 10);
	g.insert_edge(2, 1, 5);
	auto it = g.begin();
	CHECK((*it).from == 1);
	CHECK((*it).to == 2);
	CHECK((*it).weight == std::optional<int>{10});
	++it;
	CHECK((*it).from == 2);
	CHECK((*it).to == 1);
	CHECK((*it).weight == std::optional<int>{5});
}