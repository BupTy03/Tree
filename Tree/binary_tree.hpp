#pragma once

#include "registry.hpp"

#include <queue>
#include <vector>
#include <limits>
#include <cassert>
#include <algorithm>


namespace binary_tree_impl
{

	class inner_node
	{
	public:
		static constexpr auto null_node_index = std::numeric_limits<std::size_t>::max();

		std::size_t left_index() const { return left_; }
		void set_left_index(std::size_t index) { left_ = index; }
		void reset_left_index() { set_left_index(null_node_index); }

		std::size_t right_index() const { return right_; }
		void set_right_index(std::size_t index) { right_ = index; }
		void reset_right_index() { set_right_index(null_node_index); }

	private:
		std::size_t left_ = null_node_index;
		std::size_t right_ = null_node_index;
	};


	template<typename T>
	class inner_data_node : public inner_node
	{
	public:
		template<typename... Args>
		explicit inner_data_node(Args&&... args)
			: value_{std::forward<Args>(args)...}
		{}

		T& value() { return value_; }
		const T& value() const { return value_; }

	private:
		T value_;
	};

}


template<typename T>
class binary_tree
{
public:

	class node
	{
		friend class binary_tree;
		explicit node(std::size_t index) : nodeIndex_{ index } {}
	public:
		std::size_t index() const { return nodeIndex_; }
		bool is_null() const { return nodeIndex_ == binary_tree_impl::inner_node::null_node_index; }

		static node null_node() { return node(binary_tree_impl::inner_node::null_node_index); }

		friend bool operator<(const node& lhs, const node& rhs) { return lhs.nodeIndex_ < rhs.nodeIndex_; }
		friend bool operator>(const node& lhs, const node& rhs) { return lhs.nodeIndex_ > rhs.nodeIndex_; }

		friend bool operator<=(const node& lhs, const node& rhs) { return lhs.nodeIndex_ <= rhs.nodeIndex_; }
		friend bool operator>=(const node& lhs, const node& rhs) { return lhs.nodeIndex_ >= rhs.nodeIndex_; }

		friend bool operator==(const node& lhs, const node& rhs) { return lhs.nodeIndex_ == rhs.nodeIndex_; }
		friend bool operator!=(const node& lhs, const node& rhs) { return lhs.nodeIndex_ != rhs.nodeIndex_; }

	private:
		std::size_t nodeIndex_;
	};

public:
	template<typename... Args>
	explicit binary_tree(Args&&... args) 
		: rootNode_{0}
	{ 
		rootNode_ = node(nodes_.emplace(std::forward<Args>(args)...)); 
	}

	node root() { return rootNode_; }

	template<typename... Args>
	node emplace_left(const node& parent, Args&&... args)
	{ 
		check_null_node(parent);
		const auto current = nodes_.emplace(std::forward<Args>(args)...);
		inner(parent).set_left_index(current);
		return node(current);
	}

	node left(const node& n) { return node(inner(n).left_index()); }
	void reset_left(const node& n) { inner(n).reset_left_index(); }
	void set_left(const node& parent, const node& left) { inner(parent).set_left_index(left.index()); }

	template<typename... Args>
	node emplace_right(const node& parent, Args&&... args)
	{
		check_null_node(parent);
		const auto current = nodes_.emplace(std::forward<Args>(args)...);
		inner(parent).set_right_index(current);
		return node(current);
	}

	node right(const node& n) { return node(inner(n).right_index()); }
	void reset_right(const node& n) { inner(n).reset_right_index(); }
	void set_right(const node& parent, const node& right) { inner(parent).set_right_index(right.index()); }

	const T& value(const node& n) const { return inner(n).value(); }
	T& value(const node& n) { return const_cast<T&>(const_cast<const binary_tree*>(this)->value(n)); }

private:
	void check_null_node(const node& n) const { if (n.is_null()) throw std::runtime_error("node was null");  }

	const binary_tree_impl::inner_data_node<T>& inner(const node& n) const { check_null_node(n); return nodes_.value(n.index()); }
	binary_tree_impl::inner_data_node<T>& inner(const node& n)
	{
		return const_cast<binary_tree_impl::inner_data_node<T>&>(const_cast<const binary_tree*>(this)->inner(n));
	}

private:
	node rootNode_;
	registry<binary_tree_impl::inner_data_node<T>> nodes_;
};


template<typename T, typename Func>
void traverse_preorder(binary_tree<T>& tree, const typename binary_tree<T>::node& rootNode, Func func)
{
	using node_type = typename binary_tree<T>::node;

	std::vector<node_type> nodes;
	nodes.emplace_back(rootNode);

	while (!nodes.empty())
	{
		node_type current = nodes.back();
		nodes.pop_back();
		
		if (current.is_null())
			continue;

		func(tree.value(current));

		nodes.emplace_back(tree.right(current));
		nodes.emplace_back(tree.left(current));
	}
}

template<typename T, typename Func>
void traverse_preorder_recursive(binary_tree<T>& tree, const typename binary_tree<T>::node& rootNode, Func func)
{
	if (rootNode.is_null()) return;

	func(tree.value(rootNode));
	traverse_preorder_recursive(tree, tree.left(rootNode), func);
	traverse_preorder_recursive(tree, tree.right(rootNode), func);
}

template<typename T, typename Func>
void morris_traversal_preorder(binary_tree<T>& tree, typename binary_tree<T>::node rootNode, Func func)
{
	while (!rootNode.is_null())
	{
		auto left = tree.left(rootNode);
		if (left.is_null())
		{
			func(tree.value(rootNode));
			rootNode = tree.right(rootNode);
		}
		else
		{
			// Find inorder predecessor
			auto predecessor = left;
			auto right = tree.right(predecessor);
			while (!(right.is_null() || right == rootNode))
			{
				predecessor = right;
				right = tree.right(right);
			}
			
			if (right == rootNode)
			{
				// If the right child of inorder predecessor 
				// already points to this node
				tree.reset_right(predecessor);
				rootNode = tree.right(rootNode);
			}
			else
			{
				// If right child doesn't point to this node, then print this  
				// node and make right child point to this node
				func(tree.value(rootNode));
				tree.set_right(predecessor, rootNode);
				rootNode = tree.left(rootNode);
			}
		}
	}
}

template<typename T, typename Func>
void traverse_inorder(binary_tree<T>& tree, typename binary_tree<T>::node current, Func func)
{
	using node_type = typename binary_tree<T>::node;

	std::vector<node_type> nodes;
	while (!(current.is_null() && nodes.empty()))
	{
		if (!nodes.empty())
		{
			current = nodes.back();
			nodes.pop_back();

			func(tree.value(current));

			current = tree.right(current);
		}

		while (!current.is_null())
		{
			nodes.emplace_back(current);
			current = tree.left(current);
		}
	}
}

template<typename T, typename Func>
void morris_traversal_inorder(binary_tree<T>& tree, typename binary_tree<T>::node rootNode, Func func)
{
	while (!rootNode.is_null())
	{
		auto left = tree.left(rootNode);
		if (left.is_null())
		{
			func(tree.value(rootNode));
			rootNode = tree.right(rootNode);
		}
		else
		{
			// Find the inorder predecessor of current
			auto predecessor = left;
			auto right = tree.right(predecessor);
			while (!(right.is_null() || right == rootNode))
			{
				predecessor = right;
				right = tree.right(right);
			}

			if (right.is_null())
			{
				// Make current as the right child of its inorder predecessor
				tree.set_right(predecessor, rootNode);
				rootNode = tree.left(rootNode);
			}
			else
			{
				// Revert the changes made in the 'if' part to restore
				// the original tree i.e., fix the right child of predecessor
				tree.reset_right(predecessor);
				func(tree.value(rootNode));
				rootNode = tree.right(rootNode);
			}
		}
	}
}

template<typename T, typename Func>
void traverse_inorder_recursive(binary_tree<T>& tree, const typename binary_tree<T>::node& rootNode, Func func)
{
	if (rootNode.is_null()) return;

	traverse_inorder(tree, tree.left(rootNode), func);
	func(tree.value(rootNode));
	traverse_inorder(tree, tree.right(rootNode), func);
}

template<typename T, typename Func>
void traverse_postorder(binary_tree<T>& tree, typename binary_tree<T>::node current, Func func)
{
	using node_type = typename binary_tree<T>::node;

	std::vector<node_type> nodes;
	while (!(current.is_null() && nodes.empty()))
	{
		if (!nodes.empty())
		{
			current = nodes.back();
			nodes.pop_back();
			node_type right = tree.right(current);
			if (!nodes.empty() && right == nodes.back())
			{
				current = right;
				nodes.pop_back();
			}
			else
			{
				func(tree.value(current));
				current = node_type::null_node();
			}
		}

		while (!current.is_null())
		{
			nodes.emplace_back(current);
			node_type right = tree.right(current);
			if (!right.is_null())
			{
				nodes.emplace_back(right);
				nodes.emplace_back(current);
			}

			current = tree.left(current);
		}
	}
}

template<typename T, typename Func>
void traverse_postorder_recursive(binary_tree<T>& tree, const typename binary_tree<T>::node& rootNode, Func func)
{
	if (rootNode.is_null()) return;

	traverse_postorder_recursive(tree, tree.left(rootNode), func);
	traverse_postorder_recursive(tree, tree.right(rootNode), func);
	func(tree.value(rootNode));
}

template<typename T, typename Func>
void traverse_depth_first(binary_tree<T>& tree, const typename binary_tree<T>::node& root, Func func)
{
	using node_type = typename binary_tree<T>::node;

	if (root.is_null()) return;

	std::queue<node_type> children;
	children.emplace(root);
	while (!children.empty())
	{
		node_type current = children.front();
		children.pop();

		func(tree.value(current));

		node_type left = tree.left(current);
		if (!left.is_null()) children.emplace(left);

		node_type right = tree.right(current);
		if (!right.is_null()) children.emplace(right);
	}
}
