#pragma once

#include <vector>
#include <limits>
#include <cassert>
#include <stack>


namespace binary_tree_impl
{

	class inner_node
	{
	public:
		static constexpr auto null_node_index = std::numeric_limits<std::size_t>::max();

		inner_node()
			: parent_{ null_node_index }
			, left_{ null_node_index }
			, right_{ null_node_index }
		{}

		explicit inner_node(std::size_t parent)
			: parent_{parent}
			, left_{ null_node_index }
			, right_{ null_node_index }
		{}

		std::size_t parent_index() const { return parent_; }

		std::size_t left_index() const { return left_; }
		void set_left_index(std::size_t index) { left_ = index; }
		void reset_left_index() { set_left_index(null_node_index); }

		std::size_t right_index() const { return right_; }
		void set_right_index(std::size_t index) { right_ = index; }
		void reset_right_index() { set_right_index(null_node_index); }

		bool is_root() const { return parent_ == null_node_index; }

	private:
		std::size_t parent_;
		std::size_t left_;
		std::size_t right_;
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

	class node_base
	{
	public:
		explicit node_base(std::size_t index)
			: nodeIndex_{ index }
		{}

		std::size_t index() const { return nodeIndex_; }
		bool is_null() const { return nodeIndex_ == inner_node::null_node_index; }
		bool is_root() const { return nodeIndex_ == 0; }

	private:
		std::size_t nodeIndex_;
	};

}


template<typename T>
class binary_tree
{
	using inner_node = binary_tree_impl::inner_node;
	using inner_data_node = binary_tree_impl::inner_data_node<T>;
public:

	class node : public binary_tree_impl::node_base
	{
		friend class binary_tree;
		explicit node(std::size_t index) : binary_tree_impl::node_base(index) { }
	};

public:
	template<typename... Args>
	explicit binary_tree(Args&&... args) { nodes_.emplace_back(std::forward<Args>(args)...); }

	node root() { return node(0); }

	template<typename... Args>
	node emplace_left(const node& parent, Args&&... args)
	{ 
		nodes_.emplace_back(std::forward<Args>(args)...);
		const auto lastIndex = nodes_.size() - 1;
		inner(parent).set_left_index(lastIndex);
		return node(lastIndex);
	}

	node left(const node& n) { return node(inner(n).left_index()); }
	void reset_left(const node& n) { return node(inner(n).reset_left_index()); }
	void set_left(const node& n, const node& left) { return node(inner(n).set_left_index(left)); }

	template<typename... Args>
	node emplace_right(const node& parent, Args&&... args)
	{
		nodes_.emplace_back(std::forward<Args>(args)...);
		const auto lastIndex = nodes_.size() - 1;
		inner(parent).set_right_index(lastIndex);
		return node(lastIndex);
	}

	node right(const node& n) { return node(inner(n).right_index()); }
	void reset_right(const node& n) { return node(inner(n).reset_right_index()); }
	void set_right(const node& n, const node& right) { return node(inner(n).set_right_index(right)); }

	node parent(const node& n)
	{
		check_null_node(n);
		if(n.is_root())
			throw std::runtime_error{ "root node has no parent" };

		return node(inner(n).parent_index());
	}

	void reset_parent(const node& n){ return node(inner(n).reset_parent_index()); }
	void set_parent(const node& n, const node& parent) { return node(inner(n).set_parent_index(parent)); }

	const T& node_value(const node& n) const { return inner(n).value(); }
	T& node_value(const node& n) { return const_cast<T&>(const_cast<const binary_tree*>(this)->node_value(n)); }

private:
	void check_null_node(const node& n) const { if (n.is_null()) throw std::runtime_error{ "node was null" };  }

	const inner_data_node& inner(const node& n) const { check_null_node(n); return nodes_.at(n.index()); }
	inner_data_node& inner(const node& n)
	{
		return const_cast<inner_data_node&>(const_cast<const binary_tree*>(this)->inner(n));
	}

private:
	std::vector<inner_data_node> nodes_;
};


template<typename T, typename Func>
void traverse_preorder(binary_tree<T>& tree, const typename binary_tree<T>::node& rootNode, Func func)
{
	using node_type = typename binary_tree<T>::node;

	std::stack<node_type> nodes;
	nodes.emplace(rootNode);

	while (!nodes.empty())
	{
		auto current = nodes.top();
		nodes.pop();
		
		if (current.is_null())
			continue;

		func(tree.node_value(current));

		nodes.emplace(tree.right(current));
		nodes.emplace(tree.left(current));
	}
}

template<typename T, typename Func>
void traverse_inorder(binary_tree<T>& tree, const typename binary_tree<T>::node& rootNode, Func func)
{
	using node_type = typename binary_tree<T>::node;

	if (rootNode.is_null())
		return;

	traverse_inorder(tree, tree.left(rootNode), func);
	func(tree.node_value(rootNode));
	traverse_inorder(tree, tree.right(rootNode), func);
}
