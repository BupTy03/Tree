#pragma once

#include "registry.hpp"

#include <queue>
#include <vector>
#include <limits>
#include <cassert>
#include <utility>
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
		: root_{0}
	{ 
		root_ = node(nodes_.emplace(std::forward<Args>(args)...)); 
	}

	node root() const { return root_; }
	void set_root(const node& n) { root_ = n; }
	
	void remove_node(const node& n) { nodes_.erase(n.index()); }

	template<typename... Args>
	node emplace_root(Args&&... args)
	{
		root_ = node(nodes_.emplace(std::forward<Args>(args)...));
		return root_;
	}

	template<typename... Args>
	node emplace_left(const node& parent, Args&&... args)
	{ 
		check_null_node(parent);
		const auto current = nodes_.emplace(std::forward<Args>(args)...);
		inner(parent).set_left_index(current);
		return node(current);
	}

	node left(const node& n) const { return node(inner(n).left_index()); }
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

	node right(const node& n) const { return node(inner(n).right_index()); }
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
	node root_;
	registry<binary_tree_impl::inner_data_node<T>> nodes_;
};


template<typename T, typename Func>
void traverse_preorder(const binary_tree<T>& tr, typename binary_tree<T>::node root, Func&& func)
{
	using node_type = typename binary_tree<T>::node;

	std::vector<node_type> nodes;
	nodes.emplace_back(root);

	while (!nodes.empty())
	{
		node_type current = nodes.back();
		nodes.pop_back();
		
		if (current.is_null())
			continue;

		func(tr.value(current));

		nodes.emplace_back(tr.right(current));
		nodes.emplace_back(tr.left(current));
	}
}

template<typename T, typename Func>
void traverse_preorder(binary_tree<T>& tr, typename binary_tree<T>::node root, Func&& func)
{
	traverse_preorder(const_cast<const binary_tree<T>&>(tr), root,
		[f = std::forward<Func>(func) ](const T& value) { f(const_cast<T&>(value)); });
}


template<typename T, typename Func>
void traverse_preorder_recursive(const binary_tree<T>& tr, typename binary_tree<T>::node root, Func func)
{
	if (root.is_null()) return;

	func(tr.value(root));
	traverse_preorder_recursive(tr, tr.left(root), func);
	traverse_preorder_recursive(tr, tr.right(root), func);
}

template<typename T, typename Func>
void traverse_preorder_recursive(binary_tree<T>& tr, typename binary_tree<T>::node root, Func func)
{
	traverse_preorder_recursive(const_cast<const binary_tree<T>&>(tr), root, 
		[func](const T& value) { func(const_cast<T&>(value)); });
}


template<typename T, typename Func>
void morris_traversal_preorder(binary_tree<T>& tr, typename binary_tree<T>::node root, Func&& func)
{
	while (!root.is_null())
	{
		auto left = tr.left(root);
		if (left.is_null())
		{
			func(tr.value(root));
			root = tr.right(root);
		}
		else
		{
			// Find inorder predecessor
			auto predecessor = left;
			auto right = tr.right(predecessor);
			while (!(right.is_null() || right == root))
			{
				predecessor = right;
				right = tr.right(right);
			}
			
			if (right == root)
			{
				// If the right child of inorder predecessor 
				// already points to this node
				tr.reset_right(predecessor);
				root = tr.right(root);
			}
			else
			{
				// If right child doesn't point to this node, then print this  
				// node and make right child point to this node
				func(tr.value(root));
				tr.set_right(predecessor, root);
				root = tr.left(root);
			}
		}
	}
}


template<typename T, typename Func>
void traverse_inorder(const binary_tree<T>& tr, typename binary_tree<T>::node root, Func&& func)
{
	using node_type = typename binary_tree<T>::node;

	std::vector<node_type> nodes;
	while (!(root.is_null() && nodes.empty()))
	{
		if (!nodes.empty())
		{
			root = nodes.back();
			nodes.pop_back();

			func(tr.value(root));

			root = tr.right(root);
		}

		while (!root.is_null())
		{
			nodes.emplace_back(root);
			root = tr.left(root);
		}
	}
}

template<typename T, typename Func>
void traverse_inorder(binary_tree<T>& tr, typename binary_tree<T>::node root, Func&& func)
{
	traverse_inorder(const_cast<const binary_tree<T>&>(tr), root,
		[f = std::forward<Func>(func)](const T& value) { f(const_cast<T&>(value)); });
}


template<typename T, typename Func>
void morris_traversal_inorder(binary_tree<T>& tr, typename binary_tree<T>::node root, Func&& func)
{
	while (!root.is_null())
	{
		auto left = tr.left(root);
		if (left.is_null())
		{
			func(tr.value(root));
			root = tr.right(root);
		}
		else
		{
			// Find the inorder predecessor of current
			auto predecessor = left;
			auto right = tr.right(predecessor);
			while (!(right.is_null() || right == root))
			{
				predecessor = right;
				right = tr.right(right);
			}

			if (right.is_null())
			{
				// Make current as the right child of its inorder predecessor
				tr.set_right(predecessor, root);
				root = tr.left(root);
			}
			else
			{
				// Revert the changes made in the 'if' part to restore
				// the original tree i.e., fix the right child of predecessor
				tr.reset_right(predecessor);
				func(tr.value(root));
				root = tr.right(root);
			}
		}
	}
}


template<typename T, typename Func>
void traverse_inorder_recursive(const binary_tree<T>& tr, const typename binary_tree<T>::node& root, Func func)
{
	if (root.is_null()) return;

	traverse_inorder(tr, tr.left(root), func);
	func(tr.value(root));
	traverse_inorder(tr, tr.right(root), func);
}

template<typename T, typename Func>
void traverse_inorder_recursive(binary_tree<T>& tr, const typename binary_tree<T>::node& root, Func func)
{
	traverse_inorder_recursive(const_cast<const binary_tree<T>&>(tr), root,
		[func](const T& value) { func(const_cast<T&>(value)); });
}


template<typename T, typename Func>
void traverse_postorder(const binary_tree<T>& tr, typename binary_tree<T>::node root, Func&& func)
{
	using node_type = typename binary_tree<T>::node;

	std::vector<node_type> nodes;
	while (!(root.is_null() && nodes.empty()))
	{
		if (!nodes.empty())
		{
			root = nodes.back();
			nodes.pop_back();
			node_type right = tr.right(root);
			if (!nodes.empty() && right == nodes.back())
			{
				root = right;
				nodes.pop_back();
			}
			else
			{
				func(tr.value(root));
				root = node_type::null_node();
			}
		}

		while (!root.is_null())
		{
			nodes.emplace_back(root);
			node_type right = tr.right(root);
			if (!right.is_null())
			{
				nodes.emplace_back(right);
				nodes.emplace_back(root);
			}

			root = tr.left(root);
		}
	}
}

template<typename T, typename Func>
void traverse_postorder(binary_tree<T>& tr, typename binary_tree<T>::node root, Func&& func)
{
	traverse_postorder(const_cast<const binary_tree<T>&>(tr), root,
		[f = std::forward<Func>(func)](const T& value) { f(const_cast<T&>(value)); });
}


template<typename T, typename Func>
void morris_traversal_postorder(binary_tree<T>& tr, Func&& func)
{
	const auto r = tr.root();
	if (r.is_null()) return;

	auto root = tr.emplace_root(T());
	tr.set_left(root, r);

	while (!root.is_null())
	{
		auto left = tr.left(root);
		if (left.is_null())
		{
			root = tr.right(root);
		}
		else
		{
			// Find the inorder predecessor of current
			auto predecessor = left;
			auto right = tr.right(predecessor);
			while (!(right.is_null() || right == root))
			{
				predecessor = right;
				right = tr.right(right);
			}

			if (right.is_null())
			{
				// Make current as the right child of its inorder predecessor
				tr.set_right(predecessor, root);
				root = tr.left(root);
			}
			else
			{
				// predecessor found second time
				// reverse the right refernces in chain from pred to p
				auto first = root;
				auto middle = tr.left(root);
				while (middle != root)
				{
					auto last = tr.right(middle);
					tr.set_right(middle, first);
					first = middle;
					middle = last;
				}

				// visit the nodes from pred to p
				// again reverse the right references from pred to p
				first = root;
				middle = predecessor;
				while (middle != root)
				{
					func(tr.value(middle));
					auto last = tr.right(middle);
					tr.set_right(middle, first);
					first = middle;
					middle = last;
				}

				// Revert the changes made in the 'if' part to restore
				// the original tree i.e., fix the right child of predecessor
				tr.reset_right(predecessor);
				root = tr.right(root);
			}
		}
	}

	tr.remove_node(root);
	tr.set_root(r);
}


template<typename T, typename Func>
void traverse_postorder_recursive(const binary_tree<T>& tr, typename binary_tree<T>::node root, Func func)
{
	if (root.is_null()) return;

	traverse_postorder_recursive(tr, tr.left(root), func);
	traverse_postorder_recursive(tr, tr.right(root), func);
	func(tr.value(root));
}

template<typename T, typename Func>
void traverse_postorder_recursive(binary_tree<T>& tr, typename binary_tree<T>::node root, Func func)
{
	traverse_postorder_recursive(const_cast<const binary_tree<T>&>(tr), root,
		[func](const T& value) { func(const_cast<T&>(value)); });
}


template<typename T, typename Func>
void traverse_depth_first(const binary_tree<T>& tr, typename binary_tree<T>::node root, Func&& func)
{
	using node_type = typename binary_tree<T>::node;

	if (root.is_null()) return;

	std::queue<node_type> children;
	children.emplace(root);
	while (!children.empty())
	{
		node_type current = children.front();
		children.pop();

		func(tr.value(current));

		node_type left = tr.left(current);
		if (!left.is_null()) children.emplace(left);

		node_type right = tr.right(current);
		if (!right.is_null()) children.emplace(right);
	}
}

template<typename T, typename Func>
void traverse_depth_first(binary_tree<T>& tr, typename binary_tree<T>::node root, Func&& func)
{
	traverse_depth_first(const_cast<const binary_tree<T>&>(tr), root,
		[f = std::forward<Func>(func)](const T& value) { f(const_cast<T&>(value)); });
}

