
cmake_minimum_required(VERSION 3.5)
project(planix)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

add_subdirectory(external/coreutils)
add_subdirectory(external/mosquitto)
add_subdirectory(external/linenoise-ng)

add_executable(client src/client.cpp)
target_include_directories(client PRIVATE external/mosquitto/lib)
target_link_libraries(
  client PRIVATE coreutils linenoise libmosquitto pthread)


