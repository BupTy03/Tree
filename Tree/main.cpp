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

	auto r = btree.root();
	btree.emplace_left(r, "left");
	btree.emplace_right(r, "right");

	traverse_inorder(btree, r, Printer());

	return 0;
}
