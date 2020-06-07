#include "tree.hpp"

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

	auto printElem = [](const std::string& elem) { std::cout << elem << '\n'; };
	traverse_preorder(root.begin(), root.end(), printElem);
	std::cout << "\n=\n\n";
	traverse_postorder(root.begin(), root.end(), printElem);
}


int main()
{
	test_tree();
	return 0;
}
