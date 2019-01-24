#ifndef __BINARY_PARSERS_H
#define __BINARY_PARSERS_H

#include "augdwarf.h"

namespace augdw {

namespace property {
	std::string const command_name = "cmd";
	std::string const command_arguments = "args";
	std::string const label_name = "name";
}

namespace value {
	std::string const debug_info_section = ".debug_info";
	std::string const debug_abbrev_section = ".debug_abbrev";

	std::string const dwarf_section_parameters = "progbits noalloc noexec nowrite align=1";

	std::string const command_type = "cmd";
	std::string const label_type = "label";

	std::string const section_command = "section";
	std::string const db_command = "db";
	std::string const dq_command = "dq";
}

/*
 *	Obviously, ``Relocations'' parser extracts relocations.
 *	Wrapper for unoredered_map from section ids to lists of entries.
 */
class Relocations : public BinaryParser {
public:
	static int const drd_buffer_version = 2;

	struct Entry {
		Dwarf_Unsigned begin, end;
		Dwarf_Unsigned label_id;
		Entry(Dwarf_Unsigned begin, Dwarf_Unsigned end, Dwarf_Unsigned label_id)
			: begin(begin), end(end), label_id(label_id) { }
	};

	using RelocationTables = std::unordered_map<std::string, std::list<Entry>>;

private:
	int result;
	RelocationTables relocation_tables;
	std::shared_ptr<NameTable<Dwarf_Unsigned>> names;

public:
	Relocations(std::shared_ptr<NameTable<Dwarf_Unsigned>> names) : result(DW_DLV_NO_ENTRY), names(names) { }
	virtual ~Relocations() = default;

	int operator()(Dwarf_P_Debug dbg, Dwarf_Error* error, Json::Value& tree, Dwarf_Signed sections_size) override;

	int status() const { return result; }

	RelocationTables const* operator->() { return &relocation_tables; }
	RelocationTables const& operator*() const { return relocation_tables; }
};

/*
 *	Base for all classes that should create sections in JSON.
 *	Provides basic tools to ease byte manipulation.
 */
class SectionCreator {
protected:
	// 1st order lang
	Json::Value create_section(std::string section_name);
	Json::Value create_label(std::string label);
	Json::Value create_bytes(char const* begin, char const* end);
	Json::Value create_address(std::string label);

public:
	virtual ~SectionCreator() = default;
};

/*
 *	This one parses all debug_info and debug_abbrev sections.
 *	It needs table of label names for work.
 */
class ParseDwarfInfoAndAbbrev : public SectionCreator, public BinaryParser {
private:
	static std::string const abbrev_label;
	std::shared_ptr<NameTable<Dwarf_Unsigned>> names;
	std::shared_ptr<Relocations> relocations;

	// 1.5th order lang
	// Can't move to SectionCreator because of the label with drd_symbol_index == 1 (address of abbrev section)
	void add_bytes_with_relocations(
		Json::Value& array,
		char const* begin,
		char const* end,
		std::list<Relocations::Entry> const& related_relocations);

	// 2nd order lang
	void add_abbrev(
		Json::Value& array,
		char const* begin,
		char const* end);

	void add_info(
		Json::Value& array,
		char const* begin,
		char const* end);

public:
	ParseDwarfInfoAndAbbrev(std::shared_ptr<NameTable<Dwarf_Unsigned>> names, std::shared_ptr<Relocations> relocations) : names(names), relocations(relocations) { }
	virtual ~ParseDwarfInfoAndAbbrev() = default;
	int operator()(Dwarf_P_Debug dbg, Dwarf_Error* error, Json::Value& tree, Dwarf_Signed sections_size) override;
};

} // namespace augdw

#endif //__BINARY_PARSERS_H
