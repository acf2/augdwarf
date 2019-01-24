#include "binary_parsers.h"

#include <iterator>

using namespace augdw;

std::string const ParseDwarfInfoAndAbbrev::abbrev_label = "__abbrev_beginning";

void ParseDwarfInfoAndAbbrev::add_bytes_with_relocations(Json::Value& array, char const* begin, char const* end, std::list<Relocations::Entry> const& related_relocations) {
	std::list<Relocations::Entry> relocations = related_relocations;

	relocations.sort([](Relocations::Entry const& one, Relocations::Entry const& another) { return one.begin < another.begin; });

	decltype(Relocations::Entry::begin) last_position = 0;
	decltype(begin) iter = begin;
	std::string name;
	for (auto& relocation : relocations) {
		assert(relocation.end - relocation.begin == 8); // is a must

		if (last_position < relocation.begin) {
			array.append(create_bytes(iter, std::next(iter, relocation.begin - last_position)));
		}
		std::advance(iter, relocation.end - last_position);
		last_position = relocation.end;

		name = names->get_name(relocation.label_id);
		// check if special case
		if (name == value::debug_abbrev_section) {
			name = abbrev_label;
		}
		array.append(create_address(name));
	}

	if (iter != end) {
		array.append(create_bytes(iter, end));
	}
}

void ParseDwarfInfoAndAbbrev::add_abbrev(Json::Value& array, char const* begin, char const* end) {
	array.append(create_section(value::debug_abbrev_section));
	array.append(create_label(abbrev_label));

	auto it = (*relocations)->find(value::debug_abbrev_section);
	if (it != (*relocations)->end()) {
		add_bytes_with_relocations(array, begin, end, it->second);
	} else {
		array.append(create_bytes(begin, end));
	}
}

void ParseDwarfInfoAndAbbrev::add_info(Json::Value& array, char const* begin, char const* end) {
	array.append(create_section(value::debug_info_section));

	auto it = (*relocations)->find(value::debug_info_section);
	if (it != (*relocations)->end()) {
		add_bytes_with_relocations(array, begin, end, it->second);
	} else {
		array.append(create_bytes(begin, end));
	}
}

int ParseDwarfInfoAndAbbrev::operator()(Dwarf_P_Debug dbg, Dwarf_Error* error, Json::Value& tree, Dwarf_Signed sections_size) {
	int result = relocations->status();
	if (result != DW_DLV_OK) {
		return result;
	}

	assert(tree.isArray()); // This "tree" must be the root array

	Dwarf_Signed const dwarf_section = 0x99; // dummy plug
	Dwarf_Signed elf_section_index;
	Dwarf_Unsigned length;
	Dwarf_Ptr data;
	std::string section_name;

	// NOTE: There must be as many sections as section names
	for (size_t section = 0; section < sections_size; ++section) {
		result = dwarf_get_section_bytes_a(
			dbg,
			dwarf_section,
			&elf_section_index,
			&length,
			&data,
			error);
		if (result != DW_DLV_OK) {
			//TODO: Cleanup?
			return result;
		}

		// It is guaranteed in 2018-08-09 version that ``data'' will contain exactly ``length'' bytes of data
		char const* bytes = static_cast<char const*>(data);
		section_name = names->get_name(elf_section_index);

		if (section_name == value::debug_abbrev_section) {
			add_abbrev(tree, bytes, std::next(bytes, length));
		} else if (section_name == value::debug_info_section) {
			add_info(tree, bytes, std::next(bytes, length));
		} else {
			//TODO: Error handling
			return DW_DLV_NO_ENTRY;
		}
	}

	return result;
}
