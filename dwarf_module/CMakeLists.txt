project(augdwarf)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB DEBUG_MODULE_SOURCE *.cpp)

add_executable(augdwarf ${DEBUG_MODULE_SOURCE})
target_link_libraries(augdwarf jsoncpp libdwarf)
