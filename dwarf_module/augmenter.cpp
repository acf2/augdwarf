#include "augdwarf.h"

#include <iostream> //DEBUG

using namespace augdw;

int Augmenter::producer_callback(
	const char* name,
	int size,
	Dwarf_Unsigned type,
	Dwarf_Unsigned flags,
	Dwarf_Unsigned link,
	Dwarf_Unsigned info,
	Dwarf_Unsigned* sect_name_index,
	void* user_data,
	int* error)
{
	return Augmenter::global_callback(name, size, type, flags, link, info, sect_name_index, user_data, error);
}

Augmenter::CallbackType Augmenter::global_callback = [](const char* name, int size, Dwarf_Unsigned type, Dwarf_Unsigned flags, Dwarf_Unsigned link, Dwarf_Unsigned info, Dwarf_Unsigned* sect_name_index, void* user_data, int* error) -> int {
	int result = 0x31337;
	*sect_name_index = result;

	std::cout << "\tElf section number (return): 0x" << std::hex << result << std::endl;
	std::cout << "\tName: " << name << std::endl;
	std::cout << "\tSize: " << std::dec << size << std::endl;
	std::cout << "\tType: " << std::dec << type << std::endl;
	std::cout << "\tFlags: 0x" << std::hex << flags << std::endl;
	std::cout << "\tLink: " << std::dec << link << std::endl;
	std::cout << "\tSection name index: " << *sect_name_index << std::endl;
	std::cout << std::endl;

	return result;
};

Augmenter::CallbackType Augmenter::get_callback() {
	return [this](char const* name, int size, Dwarf_Unsigned type, Dwarf_Unsigned flags, Dwarf_Unsigned link, Dwarf_Unsigned info, Dwarf_Unsigned* sect_name_index, void* user_data, int* error) {
		return *sect_name_index = names->add_name(std::string(name));
	};
}

int Augmenter::init_producer() {
	Augmenter::global_callback = get_callback();

	int res = dwarf_producer_init(
		DW_DLC_WRITE | DW_DLC_SYMBOLIC_RELOCATIONS | DW_DLC_POINTER64 | DW_DLC_OFFSET64,
		&Augmenter::producer_callback,
		nullptr,
		nullptr,
		nullptr,
		"x86_64",
		"V4",
		nullptr,
		&dbg,
		&error);
	return res;
}

int Augmenter::prepare_sections() {
	return dwarf_transform_to_disk_form_a(
		dbg,
		&sections_size,
		&error);
}

int Augmenter::finish_producer() {
	return dwarf_producer_finish_a(
		dbg,
		&error);
}

int Augmenter::modify_tree(Json::Value& tree) {
	int result;

	result = init_producer();
	if (result != DW_DLV_OK) {
		// TODO: some error handling
		return result;
	}

	for (auto& feature : features) {
		assert(feature); // feature must not be empty

		result = (*feature)(tree, dbg, &error);
		if (result != DW_DLV_OK) {
			// TODO: some error handling
			return result;
		}
	}

	result = prepare_sections();
	if (result != DW_DLV_OK) {
		// TODO: some error handling
		return result;
	}

	for (auto& parser : parsers) {
		assert(parser); // parser must not be empty

		result = (*parser)(dbg, &error, tree, sections_size);
		if (result != DW_DLV_OK) {
			// TODO: some error handling
			return result;
		}
		dwarf_reset_section_bytes(dbg);
	}
	result = finish_producer();

	return result;
}
