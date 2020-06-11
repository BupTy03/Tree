#include "graph.hpp"
#include "binary_tree.hpp"

#include <iostream>
#include <iterator>
#include <algorithm>


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

struct Printer { void operator()(const std::string& elem) const { std::cout << elem << '\n'; } };

void test_binary_tree()
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
}


int main()
{
	graph<std::string> gr;

	auto center = gr.emplace_node("center");
	auto b1 = gr.emplace_neigbor(center, "block1");
	auto b2 = gr.emplace_neigbor(center, "block2");
	auto b3 = gr.emplace_neigbor(center, "block3");

	std::for_each(gr.cbegin(center), gr.cend(center), 
		[&gr](const graph_node& n) { std::cout << "node: " << gr.value_of(n) << '\n'; });

	return 0;
}
