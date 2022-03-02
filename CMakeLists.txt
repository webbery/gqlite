cmake_minimum_required(VERSION 3.0)

project(GQLlite)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3")
IF (WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
ELSEIF (APPLE)
ELSEIF (UNIX)
set(CMAKE_CXX_FLAGS "-Wno-writable-strings -Wno-comment")
include(CodeCoverage.cmake)
APPEND_COVERAGE_COMPILER_FLAGS()
setup_target_for_coverage_gcovr_xml(
	${PROJECT_NAME}_coverage NAME coverage
	EXCLUDE
		"include/json.hpp"
		"tool/gqlcmd.cpp"
		"tool/gqlexport.cpp"
		"tool/linenoise.hpp"
		"include/gql.hpp"
		"include/gql.cpp"
		"tool/getopt.c"
		"tool/getopt.h"
)
# ctest_coverage(BUILD test)
ELSE ()
ENDIF ()

add_definitions(-DGQL_LEVEL_DEBUG)
add_definitions(-DTHREAD_COUNT=4)

include_directories(./include ./libmdbx)

file(GLOB BASE_SOURCES "src/*.cpp")
aux_source_directory(src/ParserEngine ParserEngine_SOURCES)
aux_source_directory(src/StorageEngine StorageEngine_SOURCES)
aux_source_directory(src/Type Type_SOURCES)
aux_source_directory(src/Feature Feature_SOURCES)
aux_source_directory(src/gql GQL_SOURCES)
aux_source_directory(src/memory Memory_SOURCES)
aux_source_directory(src/base Struct_SOURCES)
enable_testing()
add_subdirectory(libmdbx)
add_subdirectory(test)
add_subdirectory(tool)

message("USE COMPILER: " ${CMAKE_CXX_COMPILER_ID})
add_definitions(-DYY_NO_UNISTD_H)
add_library(gqlite
	${BASE_SOURCES} ${ParserEngine_SOURCES} ${StorageEngine_SOURCES} ${Struct_SOURCES}
	${Type_SOURCES} ${Memory_SOURCES} ${Feature_SOURCES} ${GQL_SOURCES})
target_link_libraries(gqlite mdbx-static)

add_dependencies(gqlite generated_tokens generated_grammar)

message(debug: ${Type_SOURCES})
# add_custom_command(
#   TARGET gqlite
#   PRE_BUILD
#   COMMAND flex -d ${CMAKE_SOURCE_DIR}/src/gql.l
#   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
#   COMMENT "Generating lex.yy.c from gql.l"
# )
