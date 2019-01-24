#include "features.h"

int augdw::RemoveDebugInfoNode::operator()(Json::Value& tree, Dwarf_P_Debug dbg, Dwarf_Error* error) {
	using property::type;

	assert(tree.isArray());

	bool deleted = false;
	for (Json::ArrayIndex i = 0; i < tree.size(); ++i) {
		assert(tree[i].isObject());
		assert(tree[i].isMember(type));
		assert(tree[i][type].isString());
		if (tree[i][type].asString() != value::debug_node)
			continue;
		assert(!deleted); // There must be only one dwarf info node.
		Json::Value removed;
		tree.removeIndex(i, &removed);
		deleted = true;
	}

	if (!deleted)
		return DW_DLV_NO_ENTRY;

	return DW_DLV_OK;
}
