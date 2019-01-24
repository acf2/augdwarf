#include "binary_parsers.h"

using namespace augdw;

int Relocations::operator()(Dwarf_P_Debug dbg, Dwarf_Error* error, Json::Value& tree, Dwarf_Signed sections_size) {
	Dwarf_Unsigned section_count;
	int drd_buffer_version;
	result = dwarf_get_relocation_info_count(
		dbg,
		&section_count,
		&drd_buffer_version,
		error);

	if (drd_buffer_version != Relocations::drd_buffer_version) {
		result = DW_DLV_ERROR;
	}

	if (result != DW_DLV_OK) {
		// TODO: Some error handling?
		return result;
	}

	relocation_tables.clear();

	Dwarf_Signed elf_section_index, elf_section_index_link;
	Dwarf_Unsigned relocation_buffer_count;
	Dwarf_Relocation_Data relocation_data;
	std::string target_section_name;
	for (decltype(section_count) section = 0; section < section_count; ++section) {
		result = dwarf_get_relocation_info(
			dbg,
			&elf_section_index,
			&elf_section_index_link,
			&relocation_buffer_count,
			&relocation_data,
			error);

		target_section_name = names->get_name(elf_section_index_link);

		if (result != DW_DLV_OK) {
			//TODO: Error handling?
			return result;
		}

		for (decltype(relocation_buffer_count) buffer = 0; buffer < relocation_buffer_count; ++buffer) {
			assert(relocation_data[buffer].drd_length == 8);

			/*
			 *	XXX: That big switch probably should be refactored in future.
			 *	     Also, this is a switch in for in for. More than 3 indentation levels.
			 */
			switch (relocation_data[buffer].drd_type) {
			case dwarf_drt_none:
				// Not implemented
				break;
			case dwarf_drt_data_reloc:
				relocation_tables[target_section_name].emplace_back(
					relocation_data[buffer].drd_offset,
					relocation_data[buffer].drd_offset + 8,
					relocation_data[buffer].drd_symbol_index);
				break;
			case dwarf_drt_segment_rel:
				// Not implemented
				break;
			case dwarf_drt_first_of_length_pair:
				// Not implemented
				break;
			case dwarf_drt_second_of_length_pair:
				// Not implemented
				break;
			default:
				return DW_DLV_NO_ENTRY;
				break;
			}
		}
	}

	return result;
}
