#include <iostream>

#include "augdwarf.h"
#include "features.h"
#include "binary_parsers.h"
#include "dietree_handlers.h"

std::unique_ptr<augdw::Augmenter> make_augmenter() {
	using namespace augdw;

	std::shared_ptr<NameTable<Dwarf_Unsigned>> names = std::make_shared<NameTable<Dwarf_Unsigned>>();

	std::unique_ptr<Augmenter> augmenter = std::make_unique<Augmenter>(names);

	DieTree::NodeKit cu_kit = {
		value::compilation_unit,
		std::make_shared<CompilationUnitHandlerFactory>(names),
		false, // At least for now
		""
	};

	auto dietree_feature = std::make_shared<DieTree>();
	dietree_feature->add_kit(cu_kit);
	augmenter->add_feature(dietree_feature);

	augmenter->add_feature(std::make_shared<RemoveDebugInfoNode>());

	std::shared_ptr<Relocations> relocations = std::make_shared<Relocations>(names);
	augmenter->add_parser(relocations);

	std::shared_ptr<ParseDwarfInfoAndAbbrev> parse_info = std::make_shared<ParseDwarfInfoAndAbbrev>(names, relocations);
	augmenter->add_parser(parse_info);

	return std::move(augmenter);
}

int main(int argc, char* argv[]) {
	using augdw::Augmenter;

	Json::Value root;
	std::cin >> root;

	std::unique_ptr<Augmenter> augmenter = make_augmenter();
	augmenter->modify_tree(root);

	std::cout << root;
}
