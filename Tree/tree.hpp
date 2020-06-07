#pragma once

#include <vector>
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <limits>


namespace tree_impl
{

	class inner_node
	{
	public:
		void add_child(std::size_t nodeIndex)
		{
			assert(std::find(std::cbegin(children_), std::cend(children_), nodeIndex) == std::cend(children_));
			children_.emplace_back(nodeIndex);
		}

		std::vector<std::size_t>& children() { return children_; }
		const std::vector<std::size_t>& children() const { return children_; }

	private:
		std::vector<std::size_t> children_;
	};

	class inner_root_node : public inner_node
	{
	public:
		static constexpr auto index = std::numeric_limits<std::size_t>::max();
		static void has_no_value_error() { throw std::runtime_error{ "root node has no value" }; }
	};

	template<typename T>
	class inner_data_node : public inner_node
	{
	public:
		template<typename... Args>
		explicit inner_data_node(Args&&... args)
			: value_{ std::forward<Args>(args)... }
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
			: nodeIndex_{index}
		{}

		std::size_t index() const { return nodeIndex_; }
		bool is_root() const { return nodeIndex_ == inner_root_node::index; }

	private:
		std::size_t nodeIndex_;
	};

}

template<typename T>
class tree
{
	using inner_node = tree_impl::inner_node;
	using inner_root_node = tree_impl::inner_root_node;
	using inner_data_node = tree_impl::inner_data_node<T>;

public:
	class node : public tree_impl::node_base
	{
		friend class tree;

		class node_iterator;
		friend class node_iterator;

		explicit node(tree* pTree, std::size_t index)
			: tree_impl::node_base(index)
			, pTree_{ pTree }
		{
			assert(pTree_ != nullptr);
		}

	public:
		template<typename... Args>
		node emplace_child(Args&&... args)
		{
			return pTree_->emplace_child_node(index(), std::forward<Args>(args)...);
		}

		const T& operator*() const
		{
			if (is_root())
				inner_root_node::has_no_value_error();

			return static_cast<inner_data_node&>(pTree_->node_at(index())).value();
		}

		node* operator->() { return this; }
		T& operator*() { return const_cast<T&>(const_cast<const node*>(this)->operator*()); }

		class node_iterator
		{
			friend class node;

			explicit node_iterator(tree* pTree, std::vector<std::size_t>::iterator it)
				: pTree_{ pTree }
				, it_{ it }
			{
				assert(pTree_ != nullptr);
			}
		public:
			node operator*() { return node(pTree_, *it_); }
			node operator->() { return this->operator*(); }

			node_iterator& operator++() { ++it_; return *this; }
			node_iterator operator++(int) { auto it = *this; ++it_; return *it; }

			node_iterator& operator--() { --it_; return *this; }
			node_iterator operator--(int) { auto it = *this; --it_; return *it; }

			node_iterator& operator+=(std::ptrdiff_t n) { return it_ += n; }
			node_iterator& operator-=(std::ptrdiff_t n) { return it_ -= n; }

			friend node_iterator operator+(node_iterator it, std::ptrdiff_t n) { it += n; return it; }
			friend node_iterator operator-(node_iterator it, std::ptrdiff_t n) { it -= n; return it;}

			friend auto operator-(const node_iterator& lhs, const node_iterator& rhs)
			{
				assert(lhs.pTree_ == rhs.pTree_);
				return lhs.it_ - rhs.it_;
			}

			node operator[](std::ptrdiff_t n) { return *(*this + n); }

			friend bool operator==(const node_iterator& lhs, const node_iterator& rhs)
			{
				assert(lhs.pTree_ == rhs.pTree_);
				return lhs.it_ == rhs.it_;
			}
			friend bool operator!=(const node_iterator& lhs, const node_iterator& rhs) { return !(lhs == rhs); }

			friend bool operator<(const node_iterator& lhs, const node_iterator& rhs)
			{
				assert(lhs.pTree_ == rhs.pTree_);
				return lhs.it_ < rhs.it_;
			}
			friend bool operator>(const node_iterator& lhs, const node_iterator& rhs)
			{
				assert(lhs.pTree_ == rhs.pTree_);
				return lhs.it_ > rhs.it_;
			}

			friend bool operator<=(const node_iterator& lhs, const node_iterator& rhs){ return !(lhs > rhs); }
			friend bool operator>=(const node_iterator& lhs, const node_iterator& rhs) { return !(lhs < rhs); }

		private:
			tree* pTree_;
			std::vector<std::size_t>::iterator it_;
		};

		node_iterator begin() { return node_iterator(pTree_, std::begin(pTree_->node_at(index()).children())); }
		node_iterator end() { return node_iterator(pTree_, std::end(pTree_->node_at(index()).children())); }

	private:
		tree* pTree_;
	};

	friend class node;

public:
	tree() = default;
	node root() { return node(this, inner_root_node::index); }

private:
	inner_node& node_at(std::size_t index) 
	{ 
		if (index == inner_root_node::index)
			return rootNode_;

		return nodes_.at(index); 
	}
	const inner_node& node_at(std::size_t index) const 
	{ 
		return const_cast<const inner_node&>(const_cast<const tree*>(this)->node_at(index));
	}

	template<typename... Args>
	node emplace_child_node(std::size_t parent, Args&&... args)
	{
		nodes_.emplace_back(std::forward<Args>(args)...);
		const auto lastNodeIndex = nodes_.size() - 1;

		node_at(parent).add_child(lastNodeIndex);
		return node(this, lastNodeIndex);
	}

private:
	inner_root_node rootNode_;
	std::vector<inner_data_node> nodes_;
};
