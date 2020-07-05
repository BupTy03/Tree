#pragma once

#include <utility>
#include <cassert>
#include <limits>


/*
	This class is analog of original forest class 
	from: https://github.com/stlab/adobe_source_libraries/blob/master/adobe/forest.hpp
	
	This version of the forest class does not depend on the boost library. 
	It also has slight differences from the original.
*/
template<typename T>
class forest;

namespace forest_impl {

	enum class forest_edge { leading, trailing };

	template<typename NodeType>
	struct nodes_pair
	{
		nodes_pair() : prev{ nullptr }, next{ nullptr } {}
		explicit nodes_pair(NodeType* prev, NodeType* next) : prev{ prev }, next{ next } {}

		NodeType* prev;
		NodeType* next;
	};


	template<typename NodeType>
	struct forest_node_base
	{
		forest_node_base()
			: leading_(nodes_pair<NodeType>(static_cast<NodeType*>(this), static_cast<NodeType*>(this)))
			, trailing_(nodes_pair<NodeType>(static_cast<NodeType*>(this), static_cast<NodeType*>(this)))
		{}

		nodes_pair<NodeType>& get(forest_edge e) { return e == forest_edge::leading ? leading_ : trailing_; }
		const nodes_pair<NodeType>& get(forest_edge e) const { return e == forest_edge::leading ? leading_ : trailing_; }

		nodes_pair<NodeType>& leading() { return leading_; }
		nodes_pair<NodeType>& trailing() { return trailing_; }

		const nodes_pair<NodeType>& leading() const { return leading_; }
		const nodes_pair<NodeType>& trailing() const { return trailing_; }

	private:
		nodes_pair<NodeType> leading_;
		nodes_pair<NodeType> trailing_;
	};


	template<typename T>
	struct forest_node : forest_node_base<forest_node<T>>
	{
		template<typename... Args>
		explicit forest_node(Args&&... args) : value(std::forward<Args>(args)...) { }

		T value;
	};

	template<typename Iter>
	struct set_next_functor;

	template<typename T>
	class forest_const_iterator;

	template<typename T>
	class forest_iterator
	{
		using node = forest_node<T>;

		friend struct set_next_functor<forest_iterator<T>>;
		friend class forest<T>;
		friend class forest_const_iterator<T>;

		explicit forest_iterator(node* node, forest_edge edge)
			: node_{ node }, edge_{ edge } { assert(node != nullptr); }
	public:

		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = T*;
		using reference = T&;
		using iterator_category = std::bidirectional_iterator_tag;

		forest_iterator() : node_{nullptr}, edge_{ forest_edge::leading } {}

		forest_edge edge() const { return edge_; }

		bool is_leading() const { return edge_ == forest_edge::leading; }
		bool is_trailing() const { return edge_ == forest_edge::trailing; }

		void make_leading() { edge_ = forest_edge::leading; }
		void make_trailing() { edge_ = forest_edge::trailing; }

		bool equal_node(const forest_iterator& other) const { return node_ == other.edge_; }

		T& operator*() const { return node_->value; }
		T* operator->() const { return &(node_->value); }

		forest_iterator& operator++()
		{
			if (edge_ == forest_edge::leading)
			{
				node* next(node_->leading().next);
				if (next == node_) edge_ = forest_edge::trailing;
				node_ = next;
			}
			else // (edge_ == forest_edge::trailing)
			{
				node* next(node_->trailing().next);
				if(next->leading().prev == node_) edge_ = forest_edge::leading;
				node_ = next;
			}

			return *this;
		}

		forest_iterator& operator--()
		{
			if (edge_ == forest_edge::leading)
			{
				node* prev(node_->leading().prev);
				if (prev->trailing().next == node_) edge_ = forest_edge::trailing;
				node_ = prev;
			}
			else // (edge_ == forest_edge::trailing)
			{
				node* prev(node_->trailing().prev);
				if (prev == node_) edge_ = forest_edge::leading;
				node_ = prev;
			}

			return *this;
		}

		forest_iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }
		forest_iterator operator--(int) { auto tmp = *this; --(*this); return tmp; }

		friend bool operator==(const forest_iterator& lhs, const forest_iterator& rhs)
		{
			return lhs.node_ == rhs.node_ && lhs.edge_ == rhs.edge_;
		}
		friend bool operator!=(const forest_iterator& lhs, const forest_iterator& rhs)
		{
			return !(lhs == rhs);
		}

	private:
		node* node_;
		forest_edge edge_;
	};

	template<typename T>
	class forest_const_iterator
	{
		using node = forest_node<T>;

		friend struct set_next_functor<forest_const_iterator<T>>;
		friend class forest<T>;

		explicit forest_const_iterator(node* node, forest_edge edge)
			: node_{ node }, edge_{ edge } { assert(node != nullptr); }
	public:

		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = const T*;
		using reference = const T&;
		using iterator_category = std::bidirectional_iterator_tag;

		forest_const_iterator() : node_{ nullptr }, edge_{ forest_edge::leading } {}
		forest_const_iterator(forest_iterator<T> other) : node_{ other.node_ }, edge_{ other.edge_ } {}

		forest_edge edge() const { return edge_; }

		bool is_leading() const { return edge_ == forest_edge::leading; }
		bool is_trailing() const { return edge_ == forest_edge::trailing; }

		void make_leading() { edge_ = forest_edge::leading; }
		void make_trailing() { edge_ = forest_edge::trailing; }

		bool equal_node(const forest_const_iterator& other) const { return node_ == other.edge_; }

		const T& operator*() const { return node_->value; }
		const T* operator->() const { return &(node_->value); }

		forest_const_iterator& operator++()
		{
			if (edge_ == forest_edge::leading)
			{
				node* next(node_->leading().next);
				if (next == node_) edge_ = forest_edge::trailing;
				node_ = next;
			}
			else // (edge_ == forest_edge::trailing)
			{
				node* next(node_->trailing().next);
				if (next->leading().prev == node_) edge_ = forest_edge::leading;
				node_ = next;
			}

			return *this;
		}

		forest_const_iterator& operator--()
		{
			if (edge_ == forest_edge::leading)
			{
				node* prev(node_->leading().prev);
				if (prev->trailing().next == node_) edge_ = forest_edge::trailing;
				node_ = prev;
			}
			else // (edge_ == forest_edge::trailing)
			{
				node* prev(node_->trailing().prev);
				if (prev == node_) edge_ = forest_edge::leading;
				node_ = prev;
			}

			return *this;
		}

		forest_const_iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }
		forest_const_iterator operator--(int) { auto tmp = *this; --(*this); return tmp; }

		friend bool operator==(const forest_const_iterator& lhs, const forest_const_iterator& rhs)
		{
			return lhs.node_ == rhs.node_ && lhs.edge_ == rhs.edge_;
		}
		friend bool operator!=(const forest_const_iterator& lhs, const forest_const_iterator& rhs)
		{
			return !(lhs == rhs);
		}

	private:
		node* node_;
		forest_edge edge_;
	};

	template<typename Iter>
	struct set_next_functor;

	template<typename T>
	struct set_next_functor<forest_iterator<T>> {
		void operator()(forest_iterator<T> prev, forest_iterator<T> next) {
			prev.node_->get(prev.edge_).next = next.node_;
			next.node_->get(next.edge_).prev = prev.node_;
		}
	};

	template<typename T>
	struct set_next_functor<forest_const_iterator<T>> {
		void operator()(forest_const_iterator<T> prev, forest_const_iterator<T> next) {
			prev.node_->get(prev.edge_).next = next.node_;
			next.node_->get(next.edge_).prev = prev.node_;
		}
	};

	template<typename Iter>
	void set_next(Iter prev, Iter next) { set_next_functor<Iter>()(prev, next); }


	template <typename Iter>
	Iter reverse_append(Iter first, Iter last, Iter result)
	{
		while (first != last) 
		{
			Iter prior(first);
			++first;
			set_next(prior, result);
			result = prior;
		}

		return result;
	}

	template <typename Iter>
	Iter reverse_nodes(Iter first, Iter last) { return reverse_append(first, last, last); }


	template<typename Iter>
	class reverse_fullorder_iterator
	{
	public:
		using value_type = typename Iter::value_type;
		using difference_type = typename Iter::difference_type;
		using pointer = typename Iter::pointer;
		using reference = typename Iter::reference;
		using iterator_category = typename Iter::iterator_category;

		reverse_fullorder_iterator() : base_{ }, edge_{ forest_edge::leading } {}
		explicit reverse_fullorder_iterator(Iter x) 
			: base_(--x)
			, edge_(base_.is_leading() ? forest_edge::trailing : forest_edge::leading) 
		{}

		template <typename U>
		explicit reverse_fullorder_iterator(const reverse_fullorder_iterator<U>& other)
			: base_(other.base_)
			, edge_(other.edge_)
		{}

		reference operator*() const { return *base_; }
		pointer operator->() const { return &(*base_); }

		reverse_fullorder_iterator& operator++()
		{	
			if (edge_ == forest_edge::leading) 
				base_.make_trailing();
			else
				base_.make_leading();

			--base_;

			if (base_.is_leading())
				edge_ = forest_edge::trailing;
			else
				edge_ = forest_edge::leading;

			return *this;
		}
		reverse_fullorder_iterator& operator--()
		{
			if (edge_ == forest_edge::leading)
				base_.make_trailing();
			else
				base_.make_leading();

			++base_;

			if (base_.is_leading())
				edge_ = forest_edge::trailing;
			else
				edge_ = forest_edge::leading;

			return *this;
		}

		reverse_fullorder_iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }
		reverse_fullorder_iterator operator--(int) { auto tmp = *this; --(*this); return tmp; }

		friend bool operator==(const reverse_fullorder_iterator& lhs, const reverse_fullorder_iterator& rhs)
		{
			return lhs.base_ == rhs.base_ && lhs.edge_ == rhs.edge_;
		}
		friend bool operator!=(const reverse_fullorder_iterator& lhs, const reverse_fullorder_iterator& rhs)
		{
			return !(lhs == rhs);
		}

	private:
		Iter base_;
		forest_edge edge_;
	};

}


template<typename Iter>
Iter trailing_of(Iter it) { it.make_trailing(); return it; }

template<typename Iter>
Iter leading_of(Iter it) { it.make_leading(); return it; }

template <typename Iter>
bool has_children(const Iter& it) { return !it.equal_node(std::next(leading_of(it))); }

template <typename Iter>
void pivot(Iter& it) 
{
	if (it.is_leading()) it.make_trailing();
	else it.make_leading();
}

template <typename Iter>
Iter pivot_of(Iter it) { pivot(it); return it; }


template <typename Iter>
class child_iterator
{
	friend struct forest_impl::set_next_functor<child_iterator<Iter>>;

public:
	using value_type = typename Iter::value_type;
	using difference_type = typename Iter::difference_type;
	using pointer = typename Iter::pointer;
	using reference = typename Iter::reference;
	using iterator_category = typename Iter::iterator_category;


	child_iterator() = default;

	template<typename U>
	explicit child_iterator(U it) : it_{ it } {}

	reference operator*() { return *it_; }
	pointer operator->() { return &(*it_); }

	Iter base() const { return it_; }

	child_iterator& operator++()
	{
		pivot(it_);
		++it_;
		return *this;
	}

	child_iterator& operator--()
	{
		--it_;
		pivot(it_);
		return *this;
	}

	child_iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }
	child_iterator operator--(int) { auto tmp = *this; --(*this); return tmp; }

	friend bool operator==(const child_iterator& lhs, const child_iterator& rhs)
	{
		return lhs.it_ == rhs.it_;
	}

	friend bool operator!=(const child_iterator& lhs, const child_iterator& rhs)
	{
		return !(lhs == rhs);
	}

private:
	Iter it_;
};

namespace forest_impl {

	template<typename Iter>
	struct set_next_functor<child_iterator<Iter>> {
		void operator()(child_iterator<Iter> lhs, child_iterator<Iter> rhs) {
			set_next(pivot_of(lhs.base()), rhs.base());
		}
	};

}


template <typename Iter>
Iter find_edge(Iter it, forest_impl::forest_edge edge)
{
	while (it.edge() != edge) ++it;
	return it;
}

template <typename Iter>
Iter find_edge_reverse(Iter it, forest_impl::forest_edge edge)
{
	while (it.edge() != edge) --it;
	return it;
}



template <typename Iter, forest_impl::forest_edge Edge>
class edge_iterator 
{
public:
	using value_type = typename Iter::value_type;
	using difference_type = typename Iter::difference_type;
	using pointer = typename Iter::pointer;
	using reference = typename Iter::reference;
	using iterator_category = typename Iter::iterator_category;

	edge_iterator() = default;
	edge_iterator(Iter it) : it_(find_edge(it, Edge)) {}

	template <typename U>
	explicit edge_iterator(const edge_iterator<U, Edge>& u) : it_{ u.base() } {}

	reference operator*() { return *it_; }
	pointer operator->() { return &(*it_); }

	Iter base() const { return it_; }

	edge_iterator& operator++()
	{
		it_ = find_edge(std::next(it_), Edge);
		return *this;
	}

	edge_iterator& operator--()
	{
		it_ = find_edge_reverse(std::prev(it_), Edge);
		return *this;
	}

	edge_iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }
	edge_iterator operator--(int) { auto tmp = *this; --(*this); return tmp; }

	friend bool operator==(const edge_iterator& lhs, const edge_iterator& rhs)
	{
		return lhs.it_ == rhs.it_;
	}

	friend bool operator!=(const edge_iterator& lhs, const edge_iterator& rhs)
	{
		return !(lhs == rhs);
	}

private:
	Iter it_;
};


template<typename T>
class forest
{	
	using node = forest_impl::forest_node<T>;
public:
	using value_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	using iterator = forest_impl::forest_iterator<T>;
	using const_iterator = forest_impl::forest_const_iterator<T>;

	using const_child_iterator = child_iterator<const_iterator>;
	using child_iterator = child_iterator<iterator>;

	using reverse_iterator = forest_impl::reverse_fullorder_iterator<iterator>;
	using const_reverse_iterator = forest_impl::reverse_fullorder_iterator<const_iterator>;

	using preorder_iterator = edge_iterator<iterator, forest_impl::forest_edge::leading>;
	using const_preorder_iterator = edge_iterator<const_iterator, forest_impl::forest_edge::leading>;

	using postorder_iterator = edge_iterator<iterator, forest_impl::forest_edge::trailing>;
	using const_postorder_iterator = edge_iterator<const_iterator, forest_impl::forest_edge::trailing>;

	
	forest() = default;
	forest(const forest& other) : forest()
	{
		insert(end(), child_iterator(other.begin()), child_iterator(other.end()));
	}
	forest& operator=(const forest& other)
	{
		auto tmp = other;
		tmp.swap(other);
		return *this;
	}

	forest(forest&& other) noexcept : forest() { splice(end(), other); }
	forest& operator=(forest&& other) noexcept
	{
		clear();
		splice(end(), other);
		return *this;
	}

	void swap(forest& other) noexcept { std::swap(*this, other); }

	size_type size() const
	{
		if (!size_valid()) 
		{
			const_preorder_iterator first(begin());
			const_preorder_iterator last(end());

			size_ = size_type(std::distance(first, last));
		}

		return size_;
	}
	size_type max_size() const { return std::numeric_limits<size_type>::max(); }
	bool size_valid() const { return size_ != 0 || empty(); }
	bool empty() const { return begin() == end(); }
	void clear() { erase(begin(), end()); assert(empty()); }

	template<typename... Args>
	iterator emplace(const_iterator pos, Args&&... args)
	{
		iterator result(make_node(std::forward<Args>(args)...), forest_impl::forest_edge::leading);

		forest_impl::set_next(std::prev(pos), const_iterator(result));
		forest_impl::set_next(const_iterator(std::next(result)), pos);

		if (size_valid()) ++size_;

		return result;
	}
	iterator insert(const_iterator pos, const_child_iterator f, const_child_iterator l)
	{
		for (const_iterator first(f.base()), last(l.base()); first != last; ++first, ++pos)
		{
			if (first.is_leading())
				pos = insert(pos, *first);
		}

		return pos;
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		difference_type depth{ 0 };
		while (first != last)
		{
			if (first.is_leading())
			{
				++depth;
				++first;
			}
			else
			{
				if (depth > 0)
					first = erase(first);
				else
					++first;

				depth = std::max<difference_type>(0, depth - 1);
			}
		}

		return last;
	}
	iterator erase(const_iterator pos)
	{
		auto leading = leading_of(pos);
		auto leading_prev = std::prev(leading);
		auto leading_next = std::next(leading);

		auto trailing = trailing_of(pos);
		auto trailing_prev = std::prev(trailing);
		auto trailing_next = std::next(trailing);

		if (has_children(pos))
		{
			forest_impl::set_next(leading_prev, leading_next);
			forest_impl::set_next(trailing_prev, trailing_prev);
		}
		else
		{
			forest_impl::set_next(leading_prev, trailing_next);
		}

		delete_node(pos.node_);
		if (size_valid()) --size_;

		return pos.is_leading() ? std::next(leading_prev) : trailing_next;
	}

	iterator splice(const_iterator position, forest& other)
	{
		return splice(
			position, 
			other, 
			child_iterator(other.begin()), 
			child_iterator(other.end()), 
			other.size_valid() ? other.size() : 0);
	}
	iterator splice(const_iterator position, forest& other, const_iterator it)
	{
		it.make_leading();
		return splice(
			position, 
			other, 
			child_iterator(it), 
			++child_iterator(it), 
			has_children(it) ? 0 : 1);
	}
	iterator splice(iterator pos, forest& other, child_iterator first, child_iterator last, size_type count)
	{
		if (first == last || first.base() == pos) return pos;

		if (&other != this)
		{
			if (count) 
			{
				if (size_valid())
					size_ += count;

				if (other.size_valid())
					other.size_ -= count;
			}
			else 
			{
				size_ = 0;
				other.size_ = 0;
			}
		}

		iterator back(std::prev(last.base()));
		forest_impl::set_next(std::prev(first), last);
		forest_impl::set_next(std::prev(pos), first.base());
		forest_impl::set_next(back, pos);
		return first.base();
	}
	iterator splice(iterator pos, forest& other, child_iterator first, child_iterator last)
	{
		return splice(pos, other, first, last, 0);
	}
	
	iterator insert_parent(child_iterator first, child_iterator last, const T& x) 
	{
		iterator result(insert(last.base(), x));
		if (first == last) return result;

		splice(trailing_of(result), *this, first, child_iterator(result));
		return result;
	}

	void reverse(child_iterator first, child_iterator last) 
	{
		iterator prior(first.base());
		--prior;
		first = forest_impl::reverse_nodes(first, last);
		forest_impl::set_next(prior, first.base());
	}

	iterator root() { return iterator(tail(), forest_impl::forest_edge::leading); }
	const_iterator root() const { return const_iterator(tail(), forest_impl::forest_edge::leading); }

	iterator begin() { return std::next(root()); }
	iterator end() { return iterator(tail(), forest_impl::forest_edge::trailing); }

	const_iterator cbegin() const { return std::next(root()); }
	const_iterator cend() const { return const_iterator(tail(), forest_impl::forest_edge::trailing); }

	const_iterator begin() const { return cbegin(); }
	const_iterator end() const { return cend(); }

	reverse_iterator rbegin() { return reverse_iterator(end()); }
	reverse_iterator rend() { return reverse_iterator(begin()); }

	const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
	const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

	const_reverse_iterator rbegin() const { return crbegin(); }
	const_reverse_iterator rend() const { return crend(); }

private:
	template<typename... Args>
	node* make_node(Args&&... args) { return new node(std::forward<Args>(args)...); }
	void delete_node(node* n) { delete n; }

	node* tail() const { return static_cast<node*>(&tail_); }

private:
	size_type size_;
	mutable forest_impl::forest_node_base<node> tail_;
};


template <typename T>
bool operator==(const forest<T>& lhs, const forest<T>& rhs) {
	if (lhs.size() != rhs.size())
		return false;

	for (auto first(lhs.begin()), last(lhs.end()), pos(rhs.begin()); first != last; ++first, ++pos) 
	{
		if (first.edge() != pos.edge())
			return false;
		if (first.edge() && (*first != *pos))
			return false;
	}

	return true;
}

template <typename T>
bool operator!=(const forest<T>& lhs, const forest<T>& rhs) { return !(lhs == rhs); }


template <typename Iter>
child_iterator<Iter> child_begin(const Iter& it)
{
	return child_iterator<Iter>(std::next(leading_of(it)));
}

template <typename Iter>
child_iterator<Iter> child_end(const Iter& it)
{
	return child_iterator<Iter>(trailing_of(it));
}

