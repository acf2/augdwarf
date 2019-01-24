#ifndef __NAME_TABLE_H
#define __NAME_TABLE_H

#include <unordered_map>

#include "dwarf.h"

template<typename IdType>
class NameTable {
private:
	IdType new_id;
	std::unordered_map<IdType, std::string> table;

public:
	NameTable() : new_id(42) { }

	IdType add_name(std::string name) {
		table[new_id] = std::move(name);
		return new_id++;
	}
	std::string get_name(IdType id) {
		auto it = table.find(id);
		// TODO: Add error handling
		if (it == table.end())
			return "";
		return it->second;
	}
};

#endif //__NAME_TABLE_H
