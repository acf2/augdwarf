#ifndef __FEATURES_H
#define __FEATURES_H

#include <list>
#include <string>
#include <functional>
#include <unordered_map>

#include "augdwarf.h"

namespace augdw {

/*
 *	Class for handling DIE tree.
 *	It will pass special subtree of the program (under "dwarf info" node)
 *	to libdwarf producer.
 *	It uses node handlers provided by user.
 */
class DieTree : public Feature {
public:
	/*
	 *	This was made to pack/hide ``dbg'' and ``error''
	 *	They are common for all handlers and are additional complexity.
	 *	When abstracted out with handler factories, user can choose whether to use them or not to.
	 */
	class Handler {
	public:
		virtual ~Handler() = default;
		virtual int operator()(Json::Value const&, Dwarf_P_Die*) = 0;
	};

	class HandlerFactory {
	public:
		virtual ~HandlerFactory() = default;
		virtual std::unique_ptr<Handler> create(Dwarf_P_Debug dbg, Dwarf_Error* error) const = 0;
	};

	struct NodeKit {
		NameType node_type;
		std::shared_ptr<HandlerFactory> handler_factory;
		bool node_is_container; // TODO: Change to std::optional in C++17
		NameType container_property;
	};

private:
	std::list<NodeKit> node_kits;

	/*
	 *	Depth-first tree translation with use of created handlers.
	 */
	class Traverse {
	private:
		std::unordered_map<NameType, std::unique_ptr<Handler>> handler_map;
		std::unordered_map<NameType, NameType> container_map;

		Dwarf_P_Debug dbg;
		Dwarf_Error* error;

	public:
		Traverse(std::list<NodeKit> const& node_kits, Dwarf_P_Debug dbg, Dwarf_Error* error);
		int operator()(Json::Value const& node, Dwarf_P_Die* die);
	};

public:
	DieTree() = default;
	virtual ~DieTree() = default;

	void add_kit(NodeKit kit) {
		node_kits.push_back(std::move(kit));
	}

	int operator()(Json::Value& tree, Dwarf_P_Debug dbg, Dwarf_Error* error) override;
};

/*
 *	This one is self-explanatory: it simply removes debug-info node from tree.
 */
class RemoveDebugInfoNode : public Feature {
public:
	virtual ~RemoveDebugInfoNode() = default;
	int operator()(Json::Value& tree, Dwarf_P_Debug dbg, Dwarf_Error* error) override;
};

} // namespace augdw

#endif //__FEATURES_H
