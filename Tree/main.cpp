#include "tree.hpp"
#include <iostream>


int main()
{
	tree<std::string> tr;
	auto root = tr.root();

	root.emplace_child("1");
	root.emplace_child("2");
	root.emplace_child("3");

	for (auto child : root)
		std::cout << *child << std::endl;

	auto it = std::begin(root);
	it->emplace_child("4");
	it->emplace_child("5");

	for(auto child : *it)
		std::cout << *child << std::endl;

	return 0;
}
