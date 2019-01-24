
if(NOT DEFINED LIBDWARF_LOCATION)
	message(FATAL_ERROR "Path to libdwarf is not specified.")
endif()

find_package(ZLIB)

if(NOT DEFINED ZLIB_FOUND)
	message(FATAL_ERROR "Zlib is not found.")
endif()

add_library(libdwarf STATIC IMPORTED GLOBAL)

set_target_properties(libdwarf
	PROPERTIES
	IMPORTED_LOCATION "${LIBDWARF_LOCATION}/libdwarf/.libs/libdwarf.a"
	INTERFACE_INCLUDE_DIRECTORIES "${LIBDWARF_LOCATION}/libdwarf")

set_target_properties(libdwarf
	PROPERTIES
	IMPORTED_LINK_INTERFACE_LIBRARIES z)
