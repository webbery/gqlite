cmake_minimum_required(VERSION 3.0)

project(GQLlite)

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(GQLITE_BUILD_TEST    "build gqlite test"   OFF)

set(CMAKE_CXX_STANDARD 17)
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
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -fpic -ftree-vectorize -Ofast")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic -ftree-vectorize -mavx")
# if (CMAKE_SYSTEM_NAME MATCHES "Android")
# add_definitions(-DANDROID_ARM_NEON=TRUE)
# else ()
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mfma")
# endif()
ELSEIF (UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic -ftree-vectorize -fopenmp")
set(CMAKE_C_FLAGS "-Wno-writable-strings -Wno-comment")

if (GQLITE_BUILD_TEST)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -fno-omit-frame-pointer -fno-optimize-sibling-calls")
endif ()
# ANDDOID
if (CMAKE_SYSTEM_NAME MATCHES "Android")
	add_definitions(-DANDROID_ARM_NEON=TRUE)
	if (ANDROID_ABI MATCHES "armeabi-v7a")
		add_definitions(-D__ARM_ARCH_2__)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon")
		elseif (ANDROID_ABI MATCHES "armeabi-v8a")
	elseif(ANDROID_ABI MATCHES "x86_64")
		add_definitions(-D__x86_64__)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
	elseif(ANDROID_ABI MATCHES "x86")
		add_definitions(-D__x86_64__)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
	endif()
else ()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lrt -march=native -mfma -mavx")
endif()

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
	# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finstrument-functions")
endif()
# ctest_coverage(BUILD test)
ELSE ()
ENDIF ()


include(GetGitRevisionDescription.cmake)
get_git_head_revision(GIT_REFSPEC GIT_REVISION_SHA)

# find_program(VALGRIND valgrind)
# if(VALGRIND)
# 	# LY: cmake is ugly and nasty.
# 	#      - therefore memcheck-options should be defined before including ctest;
# 	#      - otherwise ctest may ignore it.
# 	set(MEMORYCHECK_SUPPRESSIONS_FILE
# 		"${CMAKE_CURRENT_SOURCE_DIR}/test/valgrind_suppress.txt"
# 		CACHE FILEPATH "Suppressions file for Valgrind" FORCE)
# 	set(MEMORYCHECK_COMMAND_OPTIONS
# 		"--suppressions=${MEMORYCHECK_SUPPRESSIONS_FILE}"
# 	# set(MEMORYCHECK_COMMAND_OPTIONS
# 	# 	"--trace-children=yes --leak-check=full --track-origins=yes --error-exitcode=42 --error-markers=@ --errors-for-leak-kinds=definite --suppressions=${MEMORYCHECK_SUPPRESSIONS_FILE}"
# 		CACHE STRING "Valgrind options" FORCE)
# 	# include(CTest)
# endif()

# get_git_unix_timestamp(GIT_REVISION_UNIX_TIMESTAMP)
# message(GIT: ${GIT_REFSPEC}, commit: ${GIT_REVISION_SHA})

add_definitions(-DPROJECT_VERSION="0.1.0")
add_definitions(-DGIT_REFSPEC=${GIT_REFSPEC} "-DGIT_REVISION_SHA=\"${GIT_REVISION_SHA}\"")

option(GQLITE_ENABLE_PRINT "print debug info" false)
if (${GQLITE_ENABLE_PRINT})
add_definitions(-DGQL_LEVEL_DEBUG)
add_definitions(-D_PRINT_FORMAT_)
endif (${GQLITE_ENABLE_PRINT})

option(GQLITE_ENABLE_COMPRESS "enable compress, but not support yet" false)
if (${GQLITE_ENABLE_COMPRESS})
add_definitions(-D_ENABLE_COMPRESS_)
endif ()

add_definitions(-DTHREAD_COUNT=4)

include_directories(./include ./third_party/libmdbx ./third_party/eigen ./third_party/fmt/include ./third_party/zstd/lib)

file(GLOB_RECURSE BASE_SOURCES "src/*.cpp")

# set(FMT_CAN_MODULE ON CACHE BOOL "Build a module instead of a traditional library." FORCE)
add_subdirectory(third_party/libmdbx)
add_subdirectory(third_party/zstd/build/cmake)
add_subdirectory(third_party/fmt)
if (GQLITE_BUILD_TEST)
if (CMAKE_SYSTEM_NAME MATCHES "Android")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funwind-tables -ffunction-sections")
endif(CMAKE_SYSTEM_NAME MATCHES "Android")
enable_testing()
add_subdirectory(test)
endif()
add_subdirectory(tool)

# message("USE COMPILER: " ${CMAKE_CXX_COMPILER_ID})
add_definitions(-DYY_NO_UNISTD_H)
add_compile_definitions(BINARY_DIR="${zstd_BINARY_DIR}/lib")
option(GQLITE_BUILD_SHARED    "build gqlite shared library"   OFF)
if (GQLITE_BUILD_SHARED)
add_library(gqlite SHARED 
	${BASE_SOURCES}
)
else()
add_library(gqlite
	${BASE_SOURCES}
)
endif()
target_link_libraries(gqlite mdbx-static fmt::fmt )

add_dependencies(gqlite zstd)
if (WIN32)
  
else (WIN32)
	if (GQLITE_BUILD_TEST)
		target_link_libraries(gqlite ${CMAKE_DL_LIBS} m)
	endif(GQLITE_BUILD_TEST)
	target_link_libraries(gqlite ${zstd_BINARY_DIR}/lib/libzstd.a)
endif(WIN32)

add_dependencies(gqlite generated_tokens generated_grammar)

install(TARGETS gqlite DESTINATION lib)
install(TARGETS gql DESTINATION bin)
install(FILES include/gqlite.h DESTINATION include)
# add_custom_command(
#   TARGET gqlite
#   PRE_BUILD
#   COMMAND flex -d ${CMAKE_SOURCE_DIR}/src/gql.l
#   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
#   COMMENT "Generating lex.yy.c from gql.l"
# )
