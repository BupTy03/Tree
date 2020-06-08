#include "tree.hpp"
#include "binary_tree.hpp"

#include <iostream>
#include <iterator>


template<typename Node>
void traverse(Node currentNode, int nestingLevel = 0)
{
	assert(nestingLevel >= 0);

	if (!currentNode.is_root())
	{
		std::cout.write("", nestingLevel);
		std::cout << *currentNode << '\n';
		++nestingLevel;
	}

	for (auto child : currentNode)
		traverse(child, nestingLevel);
}

struct Printer
{
	void operator()(const std::string& elem) const { std::cout << elem << '\n'; }
};


void test_tree()
{
	tree<std::string> tr;
	auto root = tr.root();

	root.emplace_child("1");
	root.emplace_child("2");
	root.emplace_child("3");

	auto it = std::begin(root);
	it->emplace_child("4");
	it->emplace_child("5");

	it->begin()->emplace_child("6");
	it->begin()[1]->emplace_child("7");

	traverse_preorder(root.begin(), root.end(), Printer());
	std::cout << "\n=\n\n";
	traverse_postorder(root.begin(), root.end(), Printer());
}


int main()
{
	binary_tree<std::string> btree("root");

	auto root = btree.root();
	auto left = btree.emplace_left(root, "left");
	btree.emplace_left(left, "left-left");
	btree.emplace_right(left, "left-right");

	auto right = btree.emplace_right(root, "right");
	btree.emplace_left(right, "right-left");
	btree.emplace_right(right, "right-right");

	std::cout << "====== traverse_preorder ======\n";
	traverse_preorder(btree, root, Printer());

	std::cout << "\n====== traverse_preorder_recursive ======\n";
	traverse_preorder_recursive(btree, root, Printer());

	std::cout << "\n====== traverse_inorder ======\n";
	traverse_inorder(btree, root, Printer());

	std::cout << "\n====== traverse_inorder_recursive ======\n";
	traverse_inorder_recursive(btree, root, Printer());

	std::cout << "\n====== traverse_postorder ======\n";
	traverse_postorder(btree, root, Printer());

	std::cout << "\n====== traverse_postorder_recursive ======\n";
	traverse_postorder_recursive(btree, root, Printer());

	std::cout << "\n====== traverse_depth_first ======\n";
	traverse_depth_first(btree, root, Printer());
	return 0;
}
