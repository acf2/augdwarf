#include "binary_parsers.h"

using namespace augdw;

Json::Value SectionCreator::create_section(std::string section_name) {
	Json::Value result(Json::ValueType::objectValue);

	result[property::type] = value::command_type;
	result[property::command_name] = value::section_command;
	result[property::command_arguments] = Json::Value(Json::ValueType::arrayValue);
	result[property::command_arguments].append(Json::Value(section_name + " " + value::dwarf_section_parameters));

	return result;
}

Json::Value SectionCreator::create_label(std::string label) {
	Json::Value result(Json::ValueType::objectValue);

	result[property::type] = value::label_type;
	result[property::label_name] = label;

	return result;
}

Json::Value SectionCreator::create_bytes(char const* begin, char const* end) {
	Json::Value result(Json::ValueType::objectValue);

	result[property::type] = value::command_type;
	result[property::command_name] = value::db_command;
	result[property::command_arguments] = Json::Value(Json::ValueType::arrayValue);

	for (char const* byte = begin; byte != end; ++byte) {
		result[property::command_arguments].append(static_cast<unsigned int>(*byte) & 0xFF);
	}

	return result;
}

Json::Value SectionCreator::create_address(std::string label) {
	Json::Value result(Json::ValueType::objectValue);

	result[property::type] = value::command_type;
	result[property::command_name] = value::dq_command;
	result[property::command_arguments] = Json::Value(Json::ValueType::arrayValue);
	result[property::command_arguments].append(label);

	return result;
}
