#include "features.h"

using namespace augdw;

DieTree::Traverse::Traverse(std::list<NodeKit> const& node_kits, Dwarf_P_Debug dbg, Dwarf_Error* error) : dbg(dbg), error(error) {
	for (auto& kit : node_kits) {
		if (kit.handler_factory != nullptr) {
			handler_map[kit.node_type] = std::move(kit.handler_factory->create(dbg, error));
		}

		if (kit.node_is_container) {
			container_map[kit.node_type] = kit.container_property;
		}
	}
}

int DieTree::Traverse::operator()(Json::Value const& node, Dwarf_P_Die* die) {
	using property::type;

	assert(node.isMember(type));
	assert(die != nullptr);

	int result = DW_DLV_OK;

	auto handler_iter = handler_map.find(node[type].asString());
	if (handler_iter == handler_map.end()) {
		return result;
	}

	result = (*(handler_iter->second))(node, die);
	if (result != DW_DLV_OK) {
		//TODO: Error handling
		return result;
	}

	auto container_iter = container_map.find(node[type].asString());
	if (container_iter == container_map.end()) {
		return result;
	}

	Json::Value const& container = node[container_iter->second];
	assert(container.isArray());

	Dwarf_P_Die child_die;
	for (auto& child_node : container) {
		assert(child_node.isMember(type));
		assert(child_node[type].isString());

		child_die = nullptr;
		result = (*this)(child_node, &child_die); //recursive call
		if (result != DW_DLV_OK) {
			//TODO: Error handling
			return result;
		}
		if (child_die == nullptr)
			continue;

		result = dwarf_die_link_a(child_die, *die,
			nullptr, nullptr, nullptr,
			error);
		if (result != DW_DLV_OK) {
			//TODO: Error handling
			return result;
		}
	}

	return result;
}

int DieTree::operator()(Json::Value& tree, Dwarf_P_Debug dbg, Dwarf_Error* error) {
	using property::type;
	using property::dietree;

	int result = DW_DLV_OK;
	assert(tree.isArray());

	bool found = false;
	for (Json::ArrayIndex i = 0; i < tree.size(); ++i) {
		assert(tree[i].isObject());
		assert(tree[i].isMember(type));
		assert(tree[i][type].isString());

		if (tree[i][type].asString() != value::debug_node)
			continue;

		assert(!found); // There must be only one dwarf info node.

		assert(tree[i].isMember(dietree));
		assert(tree[i][dietree].isObject()); // AFAIK there can be only one root. It cannot be a forest.

		Traverse create_tree(node_kits, dbg, error);

		Json::Value const& symroot = tree[i][dietree];
		Dwarf_P_Die dieroot;
		result = create_tree(symroot, &dieroot);
		if (result != DW_DLV_OK) {
			//TODO: some error handling
		}

		result = dwarf_add_die_to_debug_a(dbg, dieroot, error);
		found = true;
	}

	if (!found)
		return DW_DLV_NO_ENTRY;

	return result;
}
