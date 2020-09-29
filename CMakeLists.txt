
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

add_executable(client src/mqtt.cpp src/client.cpp src/server.cpp)
target_include_directories(client PRIVATE external/mosquitto/lib)
target_link_libraries(
  client PRIVATE coreutils linenoise libmosquitto pthread)

add_executable(mqtt-test src/mqtt.cpp test/mqtt.test.cpp)
target_include_directories(mqtt-test PRIVATE external/mosquitto/lib)
target_link_libraries(
        mqtt-test PRIVATE doctest::doctest coreutils libmosquitto pthread)
