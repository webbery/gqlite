cmake_minimum_required(VERSION 3.0)

project(GQLlite)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3")
IF (WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
ELSEIF (APPLE)
ELSEIF (UNIX)
set(CMAKE_C_FLAGS "-Wno-writable-strings -Wno-comment")
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
		"test/uint.hpp"
)
# ctest_coverage(BUILD test)
ELSE ()
ENDIF ()
include(GetGitRevisionDescription.cmake)
get_git_head_revision(GIT_REFSPEC GIT_REVISION_SHA)
# get_git_unix_timestamp(GIT_REVISION_UNIX_TIMESTAMP)
message(GIT: ${GIT_REFSPEC}, commit: ${GIT_REVISION_SHA})

add_definitions(-DPROJECT_VERSION="0.1.0")
add_definitions(-DGIT_REFSPEC=${GIT_REFSPEC} "-DGIT_REVISION_SHA=\"${GIT_REVISION_SHA}\"")
if (${ENABLE_PRINT})
add_definitions(-DGQL_LEVEL_DEBUG)
add_definitions(-D_PRINT_FORMAT_)
endif (${ENABLE_PRINT})

add_definitions(-DTHREAD_COUNT=4)

include_directories(./include ./third_party/libmdbx ./third_party/eigen ./third_party/fmt/include ./third_party/boost_1_78_0/include)

file(GLOB BASE_SOURCES "src/*.cpp")
aux_source_directory(src/ParserEngine ParserEngine_SOURCES)
aux_source_directory(src/StorageEngine StorageEngine_SOURCES)
aux_source_directory(src/Type Type_SOURCES)
aux_source_directory(src/Feature Feature_SOURCES)
aux_source_directory(src/gql GQL_SOURCES)
aux_source_directory(src/memory Memory_SOURCES)
aux_source_directory(src/base Struct_SOURCES)
aux_source_directory(src/operand Operand_SOURCES)
aux_source_directory(src/operand/match Match_SOURCES)
aux_source_directory(src/operand/distance Distance_SOURCES)
aux_source_directory(src/Graph Graph_SOURCES)
aux_source_directory(src/Path Path_SOURCES)

# set(FMT_CAN_MODULE ON CACHE BOOL "Build a module instead of a traditional library." FORCE)
enable_testing()
add_subdirectory(third_party/libmdbx)
add_subdirectory(third_party/fmt EXCLUDE_FROM_ALL)
add_subdirectory(test)
add_subdirectory(tool)

message("USE COMPILER: " ${CMAKE_CXX_COMPILER_ID})
add_definitions(-DYY_NO_UNISTD_H)
add_library(gqlite
	${BASE_SOURCES} ${ParserEngine_SOURCES} ${StorageEngine_SOURCES} ${Struct_SOURCES}
	${Type_SOURCES} ${Memory_SOURCES} ${Feature_SOURCES} ${GQL_SOURCES} ${Operand_SOURCES}
	${Match_SOURCES} ${Distance_SOURCES} ${Graph_SOURCES} ${Path_SOURCES})
target_link_libraries(gqlite mdbx-static fmt::fmt)

add_dependencies(gqlite generated_tokens generated_grammar)

# message(debug: ${Type_SOURCES})
# add_custom_command(
#   TARGET gqlite
#   PRE_BUILD
#   COMMAND flex -d ${CMAKE_SOURCE_DIR}/src/gql.l
#   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
#   COMMENT "Generating lex.yy.c from gql.l"
# )
