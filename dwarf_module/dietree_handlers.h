#ifndef __DIETREE_HANDLERS_H
#define __DIETREE_HANDLERS_H

#include "features.h"

using namespace augdw;

class CompilationUnitHandler : public DieTree::Handler {
private:
	Dwarf_P_Debug dbg;
	Dwarf_Error* error;
	std::shared_ptr<NameTable<Dwarf_Unsigned>> names;

	int add_filepath(Json::Value const&, Dwarf_P_Die*);
	int add_addresses(Json::Value const&, Dwarf_P_Die*);

public:
	CompilationUnitHandler(Dwarf_P_Debug dbg, Dwarf_Error* error, std::shared_ptr<NameTable<Dwarf_Unsigned>> names) : dbg(dbg), error(error), names(names) { }
	virtual ~CompilationUnitHandler() override = default;

	int operator()(Json::Value const&, Dwarf_P_Die*) override;
};

class CompilationUnitHandlerFactory : public DieTree::HandlerFactory {
private:
	std::shared_ptr<NameTable<Dwarf_Unsigned>> names;

public:
	CompilationUnitHandlerFactory(std::shared_ptr<NameTable<Dwarf_Unsigned>> names) : names(names) { }
	virtual ~CompilationUnitHandlerFactory() override = default;

	std::unique_ptr<DieTree::Handler> create(Dwarf_P_Debug dbg, Dwarf_Error* error) const override {
		return std::make_unique<CompilationUnitHandler>(dbg, error, names);
	}
};

#endif //__DIETREE_HANDLERS_H
