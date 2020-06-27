#pragma once

#include "registry.hpp"

#include <vector>
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <limits>


class tree_node
{
public:
	explicit tree_node(std::size_t index) : index_{ index } { }
	std::size_t index() const { return index_; }

	friend bool operator==(const tree_node& lhs, const tree_node& rhs) { return lhs.index() == rhs.index(); }
	friend bool operator!=(const tree_node& lhs, const tree_node& rhs) { return lhs.index() != rhs.index(); }

	friend bool operator<(const tree_node& lhs, const tree_node& rhs) { return lhs.index() < rhs.index(); }
	friend bool operator>(const tree_node& lhs, const tree_node& rhs) { return lhs.index() > rhs.index(); }

	friend bool operator<=(const tree_node& lhs, const tree_node& rhs) { return lhs.index() <= rhs.index(); }
	friend bool operator>=(const tree_node& lhs, const tree_node& rhs) { return lhs.index() >= rhs.index(); }

private:
	std::size_t index_;
};


namespace tree_impl
{

	class inner_node
	{
	public:
		void add_child(std::size_t nodeIndex)
		{
			assert(std::find(std::cbegin(children_), std::cend(children_), tree_node(nodeIndex)) == std::cend(children_));
			children_.emplace_back(nodeIndex);
		}

		std::vector<tree_node>& children() { return children_; }
		const std::vector<tree_node>& children() const { return children_; }

	private:
		std::vector<tree_node> children_;
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
class tree
{
	friend class tree_node;
public:
	using node_iterator = std::vector<tree_node>::iterator;
	using const_node_iterator = std::vector<tree_node>::const_iterator;

	template<typename... Args>
	explicit tree(Args&&... args)
		: root_{0}
		, nodes_()
	{
		root_ = tree_node(nodes_.emplace(std::forward<Args>(args)...));
	}

	tree_node root() const { return root_; }
	void set_root(const tree_node& n) { root_ = n; }

	template<typename... Args>
	tree_node emplace_child(const tree_node& parent, Args&&... args)
	{
		const auto lastNodeIndex = nodes_.emplace(std::forward<Args>(args)...);
		node_at(parent.index()).add_child(lastNodeIndex);
		return tree_node(lastNodeIndex);
	}

	void add_child(const tree_node& parent, const tree_node& child)
	{
		if (parent == child) throw std::invalid_argument("node can't be self parent");
		node_at(parent.index()).add_child(child.index());
	}

	const T& value_of(const tree_node& n) const { return nodes_.value(n.index()).value(); }
	T& value_of(const tree_node& n) { return nodes_.value(n.index()).value(); }

	node_iterator begin(const tree_node& n) { return std::begin(node_at(n.index()).children()); }
	node_iterator end(const tree_node& n) { return std::end(node_at(n.index()).children()); }

	const_node_iterator cbegin(const tree_node& n) const { return std::begin(node_at(n.index()).children()); }
	const_node_iterator cend(const tree_node& n) const { return std::end(node_at(n.index()).children()); }

	const_node_iterator begin(const tree_node& n) const { return cbegin(n); }
	const_node_iterator end(const tree_node& n) const { return cend(n); }


	class node_range
	{
		friend tree;

		explicit node_range(tree* pGraph, tree_node n)
			: pTree_{ pGraph }
			, node_{ n }
		{ assert(pTree_ != nullptr); }

	public:
		node_iterator begin() { return pTree_->begin(node_); }
		node_iterator end() { return pTree_->end(node_); }

		const_node_iterator cbegin() const { return pTree_->cbegin(node_); }
		const_node_iterator cend() const { return pTree_->cend(node_); }

	private:
		tree* pTree_;
		tree_node node_;
	};

	class const_node_range
	{
		friend tree;

		explicit const_node_range(tree* pGraph, tree_node n)
			: pTree_{ pGraph }
			, node_{ n }
		{ assert(pTree_ != nullptr); }

	public:
		const_node_iterator cbegin() const { return pTree_->cbegin(node_); }
		const_node_iterator cend() const { return pTree_->cend(node_); }

		const_node_iterator begin() const { return pTree_->begin(node_); }
		const_node_iterator end() const { return pTree_->end(node_); }

	private:
		tree* pTree_;
		tree_node node_;
	};

	node_range children_of(const tree_node& n) { return node_range(this, n); }
	const_node_range children_of(const tree_node& n) const { return const_node_range(this, n); }

private:
	const tree_impl::inner_node& node_at(std::size_t index) const { return nodes_.value(index); }
	tree_impl::inner_node& node_at(std::size_t index) { return nodes_.value(index); }

private:
	tree_node root_;
	registry<tree_impl::inner_data_node<T>> nodes_;
};
