#ifndef GDWG_GRAPH_H
#define GDWG_GRAPH_H

#include <algorithm>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace gdwg {
	template<typename N, typename E>
	class graph;

	template<typename N, typename E>
	class edge {
	 public:
		edge(N const& src, N const& dst)
		: src{src}
		, dst{dst} {}
		virtual ~edge() = default;

		virtual auto print_edge() const -> std::string = 0;
		virtual auto is_weighted() const -> bool = 0;
		virtual auto get_weight() const -> std::optional<E> = 0;
		virtual auto get_nodes() const -> std::pair<N, N> = 0;
		virtual auto operator==(edge const& other) const -> bool = 0;
		virtual auto clone() const -> std::unique_ptr<edge<N, E>> = 0;

		N src, dst;
	};

	template<typename N, typename E>
	class weighted_edge : public edge<N, E> {
	 public:
		weighted_edge(N const& src, N const& dst, E const& weight)
		: edge<N, E>{src, dst}
		, _weight{weight} {}

		auto print_edge() const -> std::string override {
			std::ostringstream oss;
			oss << this->src << " -> " << this->dst << " | W | " << _weight;
			return oss.str();
		}

		auto is_weighted() const -> bool override {
			return true;
		}

		auto get_weight() const -> std::optional<E> override {
			return _weight;
		}

		auto get_nodes() const -> std::pair<N, N> override {
			return {this->src, this->dst};
		}

		auto operator==(edge<N, E> const& other) const -> bool override {
			auto* weighted_o = dynamic_cast<weighted_edge const*>(&other);
			return weighted_o && this->src == weighted_o->src && this->dst == weighted_o->dst
			       && this->_weight == weighted_o->_weight;
		}

		auto clone() const -> std::unique_ptr<edge<N, E>> override {
			return std::make_unique<weighted_edge<N, E>>(this->src, this->dst, this->_weight);
		}

	 private:
		E _weight;
	};

	template<typename N, typename E>
	class unweighted_edge : public edge<N, E> {
	 public:
		unweighted_edge(N const& src, N const& dst)
		: edge<N, E>{src, dst} {}

		auto print_edge() const -> std::string override {
			std::ostringstream oss;
			oss << this->src << " -> " << this->dst << " | U";
			return oss.str();
		}

		auto is_weighted() const -> bool override {
			return false;
		}

		auto get_weight() const -> std::optional<E> override {
			return std::nullopt;
		}

		auto get_nodes() const -> std::pair<N, N> override {
			return {this->src, this->dst};
		}

		auto operator==(edge<N, E> const& other) const -> bool override {
			auto* unweighted_o = dynamic_cast<unweighted_edge const*>(&other);
			return unweighted_o && this->src == unweighted_o->src && this->dst == unweighted_o->dst;
		}

		auto clone() const -> std::unique_ptr<edge<N, E>> override {
			return std::make_unique<unweighted_edge<N, E>>(this->src, this->dst);
		}
	};

	template<typename N, typename E>
	class graph {
	 public:
		// Iterator class
		class iterator {
		 public:
			using value_type = struct {
				N from;
				N to;
				std::optional<E> weight;
			};
			using reference = value_type;
			using pointer = void;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::bidirectional_iterator_tag;

			// Default constructor
			iterator() = default;

			// Constructor accepting an iterator to the edges vector
			explicit iterator(typename std::vector<std::unique_ptr<edge<N, E>>>::const_iterator it)
			: _it{it} {}

			// Dereference operator
			auto operator*() -> reference {
				auto nodes = (*_it)->get_nodes();
				_current_value = {nodes.first, nodes.second, (*_it)->get_weight()};
				return _current_value;
			}

			// Pre-increment operator
			auto operator++() -> iterator& {
				++_it;
				return *this;
			}

			// Post-increment operator
			auto operator++(int) -> iterator {
				auto temp = *this;
				++(*this);
				return temp;
			}

			// Pre-decrement operator
			auto operator--() -> iterator& {
				--_it;
				return *this;
			}

			// Post-decrement operator
			auto operator--(int) -> iterator {
				auto temp = *this;
				--(*this);
				return temp;
			}

			// Equality comparison operator
			auto operator==(iterator const& other) -> bool {
				return _it == other._it;
			}

		 private:
			typename std::vector<std::unique_ptr<edge<N, E>>>::const_iterator _it;
			mutable value_type _current_value;
		};

		// Default constructor
		graph() noexcept = default;

		// Initializer list constructor
		graph(std::initializer_list<N> il)
		: _nodes(il){};

		// Range constructor
		template<typename InputIt>
		graph(InputIt first, InputIt last) {
			for (auto it = first; it != last; ++it) {
				_nodes.insert(*it);
			}
		}

		// Move constructor
		graph(graph&& other) noexcept
		: _nodes(std::move(other._nodes))
		, _edges(std::move(other._edges)){};

		// Move assignment operator
		auto operator=(graph&& other) noexcept -> graph& {
			if (this != &other) {
				_nodes = std::move(other._nodes);
				_edges = std::move(other._edges);
			}
			return *this;
		}

		// Copy constructor
		graph(graph const& other)
		: _nodes(other._nodes) {
			for (const auto& edge : other._edges) {
				_edges.push_back(edge->clone());
			}
		}

		// Copy assignment operator
		auto operator=(graph const& other) -> graph& {
			if (this != &other) {
				_nodes = other._nodes;
				_edges.clear();
				for (const auto& edge : other._edges) {
					_edges.push_back(edge->clone());
				}
			}
			return *this;
		}

		auto insert_node(N const& value) -> bool {
			return _nodes.insert(value).second;
		}

		auto insert_edge(N const& src, N const& dst, std::optional<E> weight = std::nullopt) -> bool {
			if (!is_node(src) || !is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node does "
				                         "not exist");
			}

			std::unique_ptr<edge<N, E>> new_edge;
			if (weight) {
				new_edge = std::make_unique<weighted_edge<N, E>>(src, dst, *weight);
			}
			else {
				new_edge = std::make_unique<unweighted_edge<N, E>>(src, dst);
			}

			if (std::find_if(_edges.begin(), _edges.end(), [&new_edge](const auto& edge) { return *edge == *new_edge; })
			    != _edges.end())
			{
				return false;
			}

			_edges.push_back(std::move(new_edge));
			std::sort(_edges.begin(), _edges.end(), edge_compare);
			return true;
		}

		auto replace_node(N const& old_data, N const& new_data) -> bool {
			if (!is_node(old_data)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::replace_node on a node that doesn't exist");
			}
			if (is_node(new_data)) {
				return false;
			}
			for (auto& edge : _edges) {
				if (edge->src == old_data) {
					edge->src = new_data;
				}
				if (edge->dst == old_data) {
					edge->dst = new_data;
				}
			}
			_nodes.erase(old_data);
			_nodes.insert(new_data);
			return true;
		}

		auto merge_replace_node(N const& old_data, N const& new_data) -> void {
			if (!is_node(old_data) || !is_node(new_data)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::merge_replace_node on old or new data if they "
				                         "don't exist in the graph");
			}
			for (auto& edge : _edges) {
				if (edge->src == old_data) {
					edge->src = new_data;
				}
				if (edge->dst == old_data) {
					edge->dst = new_data;
				}
			}
			_nodes.erase(old_data);

			// Remove duplicate edges
			std::sort(_edges.begin(), _edges.end(), edge_compare);
			_edges.erase(
			    std::unique(_edges.begin(), _edges.end(), [](const auto& lhs, const auto& rhs) { return *lhs == *rhs; }),
			    _edges.end());
		}

		auto erase_node(N const& value) -> bool {
			if (!is_node(value)) {
				return false;
			}
			_nodes.erase(value);
			_edges.erase(std::remove_if(_edges.begin(),
			                            _edges.end(),
			                            [&value](const auto& edge) { return edge->src == value || edge->dst == value; }),
			             _edges.end());
			return true;
		}

		auto erase_edge(N const& src, N const& dst, std::optional<E> weight = std::nullopt) -> bool {
			if (!is_node(src) || !is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::erase_edge on src or dst if they don't exist "
				                         "in the graph");
			}
			auto it = std::remove_if(_edges.begin(), _edges.end(), [&src, &dst, &weight](const auto& edge) {
				if (edge->src == src && edge->dst == dst) {
					if (weight) {
						return edge->is_weighted() && edge->get_weight() == weight;
					}
					else {
						return !edge->is_weighted();
					}
				}
				return false;
			});

			if (it != _edges.end()) {
				_edges.erase(it, _edges.end());
				return true;
			}
			return false;
		}

		auto clear() noexcept -> void {
			_nodes.clear();
			_edges.clear();
		}

		[[nodiscard]] auto is_node(N const& value) const noexcept -> bool {
			return _nodes.find(value) != _nodes.end();
		}

		[[nodiscard]] auto empty() const noexcept -> bool {
			return _nodes.empty() && _edges.empty();
		}

		[[nodiscard]] auto is_connected(N const& src, N const& dst) const -> bool {
			if (!is_node(src) || !is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::is_connected if src or dst node don't exist "
				                         "in the graph");
			}
			return std::any_of(_edges.begin(), _edges.end(), [&src, &dst](const auto& edge) {
				return edge->src == src && edge->dst == dst;
			});
		}

		[[nodiscard]] auto nodes() const noexcept -> std::vector<N> {
			return std::vector<N>(_nodes.begin(), _nodes.end());
		}

		[[nodiscard]] auto edges(N const& src, N const& dst) const -> std::vector<std::unique_ptr<edge<N, E>>> {
			if (!is_node(src) || !is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::edges if src or dst node don't exist in the "
				                         "graph");
			}

			std::vector<std::unique_ptr<edge<N, E>>> result;
			for (const auto& edge : _edges) {
				if (edge->src == src && edge->dst == dst) {
					result.push_back(edge->clone());
				}
			}

			std::sort(result.begin(), result.end(), edge_compare);

			return result;
		}

		[[nodiscard]] auto find(N const& src, N const& dst, std::optional<E> weight = std::nullopt) const -> iterator {
			auto it = std::find_if(_edges.begin(), _edges.end(), [&src, &dst, &weight](const auto& edge) {
				if (edge->src == src && edge->dst == dst) {
					if (weight) {
						return edge->is_weighted() && edge->get_weight() == weight;
					}
					else {
						return !edge->is_weighted();
					}
				}
				return false;
			});
			return iterator(it);
		}

		[[nodiscard]] auto connections(N const& src) const -> std::vector<N> {
			if (!is_node(src)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::connections if src doesn't exist in the "
				                         "graph");
			}

			std::set<N> result;
			for (const auto& edge : _edges) {
				if (edge->src == src) {
					result.insert(edge->dst);
				}
			}
			return std::vector<N>(result.begin(), result.end());
		}

		[[nodiscard]] auto begin() const -> iterator {
			return iterator(_edges.begin());
		}

		[[nodiscard]] auto end() const -> iterator {
			return iterator(_edges.end());
		}

		[[nodiscard]] auto operator==(graph const& other) const -> bool {
			if (_nodes != other._nodes || _edges.size() != other._edges.size()) {
				return false;
			}

			auto this_edges = std::vector<std::pair<N, N>>();
			auto other_edges = std::vector<std::pair<N, N>>();
			auto this_weighted_edges = std::vector<std::tuple<N, N, E>>();
			auto other_weighted_edges = std::vector<std::tuple<N, N, E>>();
			for (const auto& edge : _edges) {
				if (edge->is_weighted()) {
					this_weighted_edges.emplace_back(edge->src, edge->dst, edge->get_weight().value());
				}
				else {
					this_edges.emplace_back(edge->src, edge->dst);
				}
			}
			for (const auto& edge : other._edges) {
				if (edge->is_weighted()) {
					other_weighted_edges.emplace_back(edge->src, edge->dst, edge->get_weight().value());
				}
				else {
					other_edges.emplace_back(edge->src, edge->dst);
				}
			}
			if (this_edges != other_edges || this_weighted_edges != other_weighted_edges) {
				return false;
			}

			return true;
		}

		friend auto operator<<(std::ostream& os, graph const& g) -> std::ostream& {
			os << "\n";
			for (const auto& node : g._nodes) {
				os << node << " (\n";

				std::vector<std::string> unweighted_edges;
				std::vector<std::string> weighted_edges;

				for (const auto& edge : g._edges) {
					if (edge->src == node) {
						if (edge->is_weighted()) {
							weighted_edges.push_back("  " + edge->print_edge() + "\n");
						}
						else {
							unweighted_edges.push_back("  " + edge->print_edge() + "\n");
						}
					}
				}

				for (const auto& e : unweighted_edges) {
					os << e;
				}
				for (const auto& e : weighted_edges) {
					os << e;
				}

				os << ")\n";
			}

			return os;
		}

	 private:
		std::set<N> _nodes;
		std::vector<std::unique_ptr<edge<N, E>>> _edges;

		static auto
		edge_compare(const std::unique_ptr<edge<N, E>>& lhs, const std::unique_ptr<edge<N, E>>& rhs) noexcept -> bool {
			if (lhs->src != rhs->src) {
				return lhs->src < rhs->src;
			}
			if (lhs->dst != rhs->dst) {
				return lhs->dst < rhs->dst;
			}
			if (lhs->is_weighted() != rhs->is_weighted()) {
				return !lhs->is_weighted(); // unweighted edges come first
			}
			if (lhs->is_weighted() && rhs->is_weighted()) {
				return lhs->get_weight() < rhs->get_weight();
			}
			return false;
		}
	};
} // namespace gdwg

#endif // GDWG_GRAPH_H
