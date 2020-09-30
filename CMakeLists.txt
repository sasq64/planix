
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
add_subdirectory(external/doctest)

add_library(mqtt STATIC src/mqtt.cpp)
target_include_directories(mqtt PRIVATE external/mosquitto/lib)
target_link_libraries( mqtt PRIVATE coreutils libmosquitto)

add_executable(client src/client.cpp src/server.cpp)
target_link_libraries(client PRIVATE mqtt coreutils linenoise pthread)

add_executable(mqtt-test test/mqtt.test.cpp)
target_link_libraries( mqtt-test PRIVATE doctest::doctest coreutils mqtt)

add_executable(server-test test/server.test.cpp ../src/server.cpp)
target_link_libraries(server-test PRIVATE doctest::doctest coreutils mqtt)
