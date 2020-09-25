cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.sqlite3)

if(UNIX)
  set(EXTRA_LIBS dl pthread)
endif()

add_library(sqlite3 STATIC database.cpp sqlite3.c)
target_compile_definitions(sqlite3 PRIVATE -DSQLITE_DISABLE_LFS -DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_FTS3_PARENTHESIS)
target_include_directories(sqlite3 PUBLIC ..)
target_link_libraries(sqlite3 PRIVATE coreutils ${EXTRA_LIBS})
