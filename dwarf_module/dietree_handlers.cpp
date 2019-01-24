#include "dietree_handlers.h"

int CompilationUnitHandler::add_filepath(Json::Value const& node, Dwarf_P_Die* die) {
	using property::filepath;

	assert(node.isObject());
	assert(node.isMember(filepath));
	assert(node[filepath].isString());

	Dwarf_P_Attribute attr;

	attr = dwarf_add_AT_name(
		*die,
		CharWrapper(node[filepath].asString()),
		error);
	if (attr == reinterpret_cast<Dwarf_P_Attribute>(DW_DLV_BADADDR)) {
		//TODO: Add error handling (and cleanup?)
		return DW_DLV_ERROR;
	}

	return DW_DLV_OK;
}

// NOTE: Low_pc and high_pc must be added in a succession.
int CompilationUnitHandler::add_addresses(Json::Value const& node, Dwarf_P_Die* die) {
	using property::begin_label;
	using property::end_label;

	assert(node.isObject());
	assert(node.isMember(begin_label) && node.isMember(end_label));
	assert(node[begin_label].isString() && node[end_label].isString());

	Dwarf_Unsigned id;
	Dwarf_P_Attribute attr;

	id = names->add_name(node[begin_label].asString());
	attr = dwarf_add_AT_targ_address_b(
		dbg,
		*die,
		DW_AT_low_pc,
		value::dummy_address,
		id,
		error);
	if (attr == reinterpret_cast<Dwarf_P_Attribute>(DW_DLV_BADADDR)) {
		//TODO: Add error handling (and cleanup?)
		return DW_DLV_ERROR;
	}

	id = names->add_name(node[end_label].asString());
	attr = dwarf_add_AT_targ_address_b(
		dbg,
		*die,
		DW_AT_high_pc,
		value::dummy_address,
		id,
		error);
	if (attr == reinterpret_cast<Dwarf_P_Attribute>(DW_DLV_BADADDR)) {
		//TODO: Add error handling (and cleanup?)
		return DW_DLV_ERROR;
	}

	return DW_DLV_OK;
}

int CompilationUnitHandler::operator()(Json::Value const& node, Dwarf_P_Die* die) {
	int result = DW_DLV_OK;

	assert(node.isObject());

	result = dwarf_new_die_a(
		dbg,
		DW_TAG_compile_unit,
		nullptr, nullptr, nullptr, nullptr, // no parent, child, left and right siblings
		die,
		error);
	if (result != DW_DLV_OK) {
		//TODO: Add error handling (and cleanup?)
		return result;
	}

	result = add_filepath(node, die);
	if (result != DW_DLV_OK)
		return result;

	result = add_addresses(node, die);

	return result;
}
