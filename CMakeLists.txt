cmake_minimum_required(VERSION 3.0)

project(GQLlite)

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -openmp -march=native -fpic -ftree-vectorize -Ofast")
IF (WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
# foreach(CMAKE_TARGET_FLAG
# 	CMAKE_C_FLAGS_DEBUG_INIT
# 	CMAKE_C_FLAGS_RELEASE_INIT
# 	CMAKE_C_FLAGS_MINSIZEREL_INIT
# 	CMAKE_C_FLAGS_RELWITHDEBINFO_INIT
# 	CMAKE_CXX_FLAGS_DEBUG_INIT
# 	CMAKE_CXX_FLAGS_MINSIZEREL_INIT
# 	CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT
# 	CMAKE_CXX_FLAGS_RELEASE_INIT
# )
# 	if (${CMAKE_TARGET_FLAG} MATCHES "/MD")
# 		string(REGEX REPLACE "/MD" "/MT" ${CMAKE_TARGET_FLAG} "${${CMAKE_TARGET_FLAG}}")
# 		message(STATUS "${CMAKE_TARGET_FLAG}:${${CMAKE_TARGET_FLAG}}")
# 	endif()
# endforeach()
ELSEIF (APPLE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -openmp -march=native -fpic -ftree-vectorize")
ELSEIF (UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lrt -fpic -ftree-vectorize")
set(CMAKE_C_FLAGS "-Wno-writable-strings -Wno-comment")
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		include(CodeCoverage.cmake)
		APPEND_COVERAGE_COMPILER_FLAGS()
		setup_target_for_coverage_gcovr_xml(
			${PROJECT_NAME}_coverage NAME coverage
			EXCLUDE
				"include/json.hpp"
				"tool/*"
				"include/gql.hpp"
				"include/gql.cpp"
				"tool/getopt.c"
				"tool/getopt.h"
				"test/uint.hpp"
				"third_party/catch2/catch.hpp"
				"include/base/parallel/pam/*"
				"include/base/parallel/parlay/*"
		)
	endif()
# ctest_coverage(BUILD test)
ELSE ()
ENDIF ()
include(GetGitRevisionDescription.cmake)
get_git_head_revision(GIT_REFSPEC GIT_REVISION_SHA)

find_program(VALGRIND valgrind)
if(VALGRIND)
	# LY: cmake is ugly and nasty.
	#      - therefore memcheck-options should be defined before including ctest;
	#      - otherwise ctest may ignore it.
	set(MEMORYCHECK_SUPPRESSIONS_FILE
		"${CMAKE_CURRENT_SOURCE_DIR}/test/valgrind_suppress.txt"
		CACHE FILEPATH "Suppressions file for Valgrind" FORCE)
	set(MEMORYCHECK_COMMAND_OPTIONS
		"--suppressions=${MEMORYCHECK_SUPPRESSIONS_FILE}"
	# set(MEMORYCHECK_COMMAND_OPTIONS
	# 	"--trace-children=yes --leak-check=full --track-origins=yes --error-exitcode=42 --error-markers=@ --errors-for-leak-kinds=definite --suppressions=${MEMORYCHECK_SUPPRESSIONS_FILE}"
		CACHE STRING "Valgrind options" FORCE)
	# include(CTest)
endif()

# get_git_unix_timestamp(GIT_REVISION_UNIX_TIMESTAMP)
# message(GIT: ${GIT_REFSPEC}, commit: ${GIT_REVISION_SHA})

add_definitions(-DPROJECT_VERSION="0.1.0")
add_definitions(-DGIT_REFSPEC=${GIT_REFSPEC} "-DGIT_REVISION_SHA=\"${GIT_REVISION_SHA}\"")

option(GQLITE_ENABLE_PRINT "print debug info" false)
if (${ENABLE_PRINT})
add_definitions(-DGQL_LEVEL_DEBUG)
add_definitions(-D_PRINT_FORMAT_)
endif (${ENABLE_PRINT})

add_definitions(-DTHREAD_COUNT=4)

include_directories(./include ./third_party/libmdbx ./third_party/eigen ./third_party/fmt/include ./third_party/zstd/lib)

file(GLOB BASE_SOURCES "src/*.cpp")
aux_source_directory(src/StorageEngine StorageEngine_SOURCES)
aux_source_directory(src/Type Type_SOURCES)
aux_source_directory(src/Feature Feature_SOURCES)
aux_source_directory(src/gql GQL_SOURCES)
aux_source_directory(src/base/system System_SOURCES)
aux_source_directory(src/base/system/memory Memory_SOURCES)
aux_source_directory(src/base/lang Language_SOURCES)
aux_source_directory(src/base Struct_SOURCES)
aux_source_directory(src/operand Operand_SOURCES)
aux_source_directory(src/operand/match Match_SOURCES)
aux_source_directory(src/operand/analysis Analysis_SOURCES)
aux_source_directory(src/operand/distance Distance_SOURCES)
aux_source_directory(src/operand/query Query_SOURCES)
aux_source_directory(src/operand/query/HNSW HNSW_SOURCES)
aux_source_directory(src/plan Plan_SOURCES)
aux_source_directory(src/Graph Graph_SOURCES)
aux_source_directory(src/Path Path_SOURCES)
aux_source_directory(src/walk Walk_SOURCES)

# set(FMT_CAN_MODULE ON CACHE BOOL "Build a module instead of a traditional library." FORCE)
add_subdirectory(third_party/libmdbx)
add_subdirectory(third_party/zstd/build/cmake)
add_subdirectory(third_party/fmt)
enable_testing()
add_subdirectory(test)
add_subdirectory(tool)

# message("USE COMPILER: " ${CMAKE_CXX_COMPILER_ID})
add_definitions(-DYY_NO_UNISTD_H)
add_compile_definitions(BINARY_DIR="${zstd_BINARY_DIR}/lib")
option(GQLITE_BUILD_SHARED    "build gqlite shared library"   OFF)
if (GQLITE_BUILD_SHARED)
add_library(gqlite SHARED 
	${BASE_SOURCES} ${StorageEngine_SOURCES} ${Struct_SOURCES}
	${Type_SOURCES} ${System_SOURCES} ${Feature_SOURCES} ${GQL_SOURCES} ${Operand_SOURCES}
	${Match_SOURCES} ${Distance_SOURCES} ${Graph_SOURCES} ${Path_SOURCES} ${Analysis_SOURCES}
	${Query_SOURCES} ${HNSW_SOURCES} ${Language_SOURCES} ${Memory_SOURCES} ${Plan_SOURCES}
	${Walk_SOURCES}
)
else()
add_library(gqlite
	${BASE_SOURCES} ${StorageEngine_SOURCES} ${Struct_SOURCES}
	${Type_SOURCES} ${System_SOURCES} ${Feature_SOURCES} ${GQL_SOURCES} ${Operand_SOURCES}
	${Match_SOURCES} ${Distance_SOURCES} ${Graph_SOURCES} ${Path_SOURCES} ${Analysis_SOURCES}
	${Query_SOURCES} ${HNSW_SOURCES} ${Language_SOURCES} ${Memory_SOURCES} ${Plan_SOURCES}
	${Walk_SOURCES}
)
endif()
target_link_libraries(gqlite mdbx-static fmt::fmt )
add_dependencies(gqlite zstd)
if (WIN32)
  
else (WIN32)
	#link_directories(${zstd_BINARY_DIR}/lib)
	target_link_libraries(gqlite ${zstd_BINARY_DIR}/lib/libzstd.a)
endif(WIN32)

add_dependencies(gqlite generated_tokens generated_grammar)

# message(debug: ${Type_SOURCES})
# add_custom_command(
#   TARGET gqlite
#   PRE_BUILD
#   COMMAND flex -d ${CMAKE_SOURCE_DIR}/src/gql.l
#   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
#   COMMENT "Generating lex.yy.c from gql.l"
# )
