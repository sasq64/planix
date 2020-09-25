# -*- mode: CMAKE; -*-

cmake_minimum_required(VERSION 2.6)

project(linenoise)

# INFO
set(LINENOISE_VERSION "1.0.0" CACHE path "Linenoise version")
set(LINENOISE_DISPLAY_NAME "Linenoise-NG")
set(LINENOISE_URL_INFO_ABOUT "https://github.com/arangodb/linenoise-ng")
set(LINENOISE_CONTACT "hackers@arangodb.org")
set(LINENOISE_FRIENDLY_STRING "Linenoise NG - Linenoise Next Generation")

if(MSVC) 
  message(STATUS "Compiler type MSVC: ${CMAKE_CXX_COMPILER}")
  add_definitions("-D_CRT_SECURE_NO_WARNINGS=1")
endif()

add_library(linenoise STATIC
  src/ConvertUTF.cpp
  src/linenoise.cpp
  src/wcwidth.cpp
)
target_include_directories(linenoise PUBLIC include PRIVATE src)

# install
install(TARGETS linenoise DESTINATION lib)

# headers
install(FILES include/linenoise.h DESTINATION include)

# build example
add_executable(
  example
  tst/example.c
)

target_link_libraries(
  example
  linenoise
)

# packaging
include(CPack)

if (MSVC)
else ()
  set(CPACK_SET_DESTDIR ON)
endif ()

set(CPACK_PACKAGE_VENDOR  "ArangoDB GmbH")
set(CPACK_PACKAGE_CONTACT "info@arangodb.com")
set(CPACK_PACKAGE_VERSION "${LINENOISE_VERSION}")

set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")

set(CPACK_STRIP_FILES "ON")

set(CPACK_PACKAGE_NAME "linenoise")
set(CPACK_DEBIAN_PACKAGE_SECTION "utilities")
