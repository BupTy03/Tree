#pragma once

#include "registry.hpp"

#include <vector>
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <limits>


class graph_node
{
public:
	explicit graph_node(std::size_t index) : index_{ index } { }
	std::size_t index() const { return index_; }

	friend bool operator==(const graph_node& lhs, const graph_node& rhs) { return lhs.index() == rhs.index(); }
	friend bool operator!=(const graph_node& lhs, const graph_node& rhs) { return lhs.index() != rhs.index(); }

	friend bool operator<(const graph_node& lhs, const graph_node& rhs) { return lhs.index() < rhs.index(); }
	friend bool operator>(const graph_node& lhs, const graph_node& rhs) { return lhs.index() > rhs.index(); }

	friend bool operator<=(const graph_node& lhs, const graph_node& rhs) { return lhs.index() <= rhs.index(); }
	friend bool operator>=(const graph_node& lhs, const graph_node& rhs) { return lhs.index() >= rhs.index(); }

private:
	std::size_t index_;
};


namespace graph_impl
{

	class inner_node
	{
	public:
		void add_neighbor(std::size_t nodeIndex)
		{
			assert(std::find(std::cbegin(neighbors_), std::cend(neighbors_), graph_node(nodeIndex)) == std::cend(neighbors_));
			neighbors_.emplace_back(nodeIndex);
		}

		std::vector<graph_node>& neighbors() { return neighbors_; }
		const std::vector<graph_node>& neighbors() const { return neighbors_; }

	private:
		std::vector<graph_node> neighbors_;
	};
	
	template<typename T>
	class inner_data_node : public inner_node
	{
	public:
		template<typename... Args>
		explicit inner_data_node(Args&&... args) : value_{ std::forward<Args>(args)... } {}

		T& value() { return value_; }
		const T& value() const { return value_; }

	private:
		T value_;
	};

}


template<typename T>
class graph
{
	friend class graph_node;

	using inner_node = graph_impl::inner_node;
	using inner_data_node = graph_impl::inner_data_node<T>;
public:
	using node_iterator = std::vector<graph_node>::iterator;
	using const_node_iterator = std::vector<graph_node>::const_iterator;

	template<typename... Args>
	graph_node emplace_node(Args&&... args) 
	{ 
		return graph_node(nodes_.emplace(std::forward<Args>(args)...));
	}

	template<typename... Args>
	graph_node emplace_neigbor(const graph_node& node, Args&&... args)
	{
		const auto lastNodeIndex = nodes_.emplace(std::forward<Args>(args)...);
		node_at(node.index()).add_neighbor(lastNodeIndex);
		node_at(lastNodeIndex).add_neighbor(node.index());
		return graph_node(lastNodeIndex);
	}

	void make_neighbors(const graph_node& node, const graph_node& neighbor)
	{
		if (node == neighbor)
			throw std::invalid_argument("node can't be self neigbor");

		node_at(node.index()).add_neighbor(neighbor.index());
		node_at(neighbor.index()).add_neighbor(node.index());
	}

	const T& value_of(const graph_node& n) const { return nodes_.value(n.index()).value(); }
	T& value_of(const graph_node& n) { return nodes_.value(n.index()).value(); }

	node_iterator begin(const graph_node& n) { return std::begin(node_at(n.index()).neighbors()); }
	node_iterator end(const graph_node& n) { return std::end(node_at(n.index()).neighbors()); }

	const_node_iterator cbegin(const graph_node& n) const { return std::begin(node_at(n.index()).neighbors()); }
	const_node_iterator cend(const graph_node& n) const { return std::end(node_at(n.index()).neighbors()); }

	const_node_iterator begin(const graph_node& n) const { return cbegin(n); }
	const_node_iterator end(const graph_node& n) const { return cend(n); }


	class node_range
	{
		friend graph;

		explicit node_range(graph* pGraph, graph_node n)
			: pGraph_{pGraph}
			, node_{n}
		{ assert(pGraph_ != nullptr); }

	public:
		node_iterator begin() { return pGraph_->begin(node_); }
		node_iterator end() { return pGraph_->end(node_); }

		const_node_iterator cbegin() const { return pGraph_->cbegin(node_); }
		const_node_iterator cend() const { return pGraph_->cend(node_); }

	private:
		graph* pGraph_;
		graph_node node_;
	};

	class const_node_range
	{
		friend graph;

		explicit const_node_range(graph* pGraph, graph_node n)
			: pGraph_{ pGraph }
			, node_{ n }
		{ assert(pGraph_ != nullptr); }

	public:
		const_node_iterator cbegin() const { return pGraph_->cbegin(node_); }
		const_node_iterator cend() const { return pGraph_->cend(node_); }

		const_node_iterator begin() const { return pGraph_->begin(node_); }
		const_node_iterator end() const { return pGraph_->end(node_); }

	private:
		graph* pGraph_;
		graph_node node_;
	};


	node_range neighbors_of(const graph_node& n) { return node_range(this, n); }
	const_node_range neighbors_of(const graph_node& n) const { return const_node_range(this, n); }

private:
	const inner_node& node_at(std::size_t index) const { return nodes_.value(index); }
	inner_node& node_at(std::size_t index) { return nodes_.value(index); }

private:
	registry<inner_data_node> nodes_;
};
