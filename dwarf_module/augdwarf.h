#ifndef __AUGDWARF_H
#define __AUGDWARF_H

#include "json/json.h"
#include "libdwarf.h"
#include "dwarf.h"

#include "name_table.h"

#include <algorithm>
#include <string>
#include <list>
#include <memory>
#include <cassert>

namespace augdw {

namespace property {
	std::string const type = "type";
	std::string const dietree = "tree";
	std::string const child_nodes = "content";
	std::string const filepath = "path";
	std::string const begin_label = "begin";
	std::string const end_label = "end";
}

namespace value {
	std::string const debug_node = "dwarf info";
	std::string const compilation_unit = "compilation unit";

	uint64_t const dummy_address = 0xFECA3713DEC0ADDE; // DEADC0DE1337CAFE
}

using NameType = std::string;

/*
 *	Ugly std::string -> char* wrapper.
 *	Unfortunately, libdwarf WANTS strings to be passed as "char*" and NOT as "const char*".
 *	Meh.
 */
class CharWrapper {
private:
	std::unique_ptr<char[]> chars;

public:
	CharWrapper(std::string const& str) : chars(new char[str.size()]) {
		std::copy_n(str.c_str(), str.size(), chars.get());
	}
	operator char*() {
		return chars.get();
	}
};

/*
 *	Features are features of augmenter.
 *	They pass any kind of valuable info from JSON tree to libdwarf library.
 *	All modifications of the tree also fall into that category.
 */
class Feature {
public:
	virtual ~Feature() = default;
	virtual int operator()(Json::Value& tree, Dwarf_P_Debug dbg, Dwarf_Error* error) = 0;
};

/*
 *	Binary parsers parse binary data and create nasm mnemonics for it.
 *	They have to deal with all things that were passed to feature classes,
 *	i.e. they transform libdwarf output back to JSON tree format.
 *	For every kind of valuable DWARF info there should be a way to represent it in nasm.
 *	Binary parser is an abstraction of that way.
 */
class BinaryParser {
public:
	virtual ~BinaryParser() = default;
	virtual int operator()(Dwarf_P_Debug dbg, Dwarf_Error* error, Json::Value& tree, Dwarf_Signed) = 0;
};

/*
 *	Main class. Nothing special.
 *	Gets the tree, does something to it, gives it back.
 *	It contains collections of features and binary parsers in sequences,
 *	because features and binary parsers should be applied sequentially.
 *
 *	Rough idea:
 *		    1) libdwarf initialization
 *		    2) Apply feature 1
 *		       ...
 *		  N+1) Apply feature N
 *		  N+2) libdwarf format sections to binary
 *		  N+3) Apply binary parser 1
 *		       ...
 *		N+M+2) Apply binary parser M
 *		N+M+3) libdwarf finalization
 *
 *	There must be configuration of features and parsers somewhere before step one.
 */
class Augmenter {
public:
	using CallbackType = std::function<int(char const*, int, Dwarf_Unsigned, Dwarf_Unsigned, Dwarf_Unsigned, Dwarf_Unsigned, Dwarf_Unsigned*, void*, int*)>;

private:
	std::list<std::shared_ptr<Feature>> features;
	std::list<std::shared_ptr<BinaryParser>> parsers;
	std::shared_ptr<NameTable<Dwarf_Unsigned>> names;

	Dwarf_P_Debug dbg;
	Dwarf_Error error;

	static int producer_callback(
		const char* name,
		int size,
		Dwarf_Unsigned type,
		Dwarf_Unsigned flags,
		Dwarf_Unsigned link,
		Dwarf_Unsigned info,
		Dwarf_Unsigned* sect_name_index,
		void* user_data,
		int* error);

	/*
	 *	This is the function that will be called by callback with C interface
	 *	that is passed to library. Because library does not accept lambdas.
	 *	It is set in init_producer and must remain valid until finish_producer ends.
	 *	init_producer and finish_producer are called sequentially within modify_tree.
	 *	Therefore, the use of it in single-threaded environment is perfectly safe,
	 *	but for multiple threads a mutex must be created.
	 */
	static CallbackType global_callback;

	Dwarf_Signed sections_size;

	CallbackType get_callback();
	int init_producer();
	int prepare_sections();
	int finish_producer();

public:
	Augmenter(std::shared_ptr<NameTable<Dwarf_Unsigned>> names) : sections_size(0), names(names) { }
	~Augmenter() = default;

	int modify_tree(Json::Value& tree);

	void add_feature(std::shared_ptr<Feature> new_feature) { features.push_back(new_feature); }
	void add_parser(std::shared_ptr<BinaryParser> new_parser) { parsers.push_back(new_parser); }
};

} // namespace augdw

#endif //__AUGDWARF_H
