cmake_minimum_required(VERSION 3.8)

project(GQLlite)

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(GQLITE_BUILD_TEST    "build gqlite test"   OFF)

include(TargetArch.cmake)
target_architecture(GQLITE_CPU_ARCHITECHURE)

message("Check Compiler: " ${CMAKE_CXX_COMPILER})

if(WIN32)
  set(GQLITE_ASM_BINFMT pe)
elseif(APPLE)
  set(GQLITE_ASM_BINFMT macho)
else()
  set(GQLITE_ASM_BINFMT elf)
endif()

math(EXPR _bits "${CMAKE_SIZEOF_VOID_P}*8")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^[Aa][Rr][Mm]" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
  set(GQLITE_ASM_ABI aapcs)
elseif(WIN32)
  set(GQLITE_ASM_ABI ms)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^mips")
	if(_bits EQUAL 32)
    set(GQLITE_ASM_ABI o32)
  else()
    set(GQLITE_ASM_ABI n64)
  endif()
else()
  set(GQLITE_ASM_ABI sysv)
endif()

set(GQLITE_ALL_ARCHS arm arm64 loongarch64 mips32 mips64 ppc32 ppc64 riscv64 s390x i386 x86_64 combined)
if(CMAKE_SYSTEM_PROCESSOR IN_LIST GQLITE_ALL_ARCHS)
  set(GQLITE_CPU_ARCHITECHURE ${CMAKE_SYSTEM_PROCESSOR})
elseif(_bits EQUAL 32)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "^[Aa][Rr][Mm]")
    set(GQLITE_CPU_ARCHITECHURE arm)
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^mips")
    set(GQLITE_CPU_ARCHITECHURE mips32)
  else()
    set(GQLITE_CPU_ARCHITECHURE i386)
  endif()
else()
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" OR
    CMAKE_SYSTEM_PROCESSOR MATCHES "^[Aa][Rr][Mm]") # armv8
    set(GQLITE_CPU_ARCHITECHURE arm64)
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^mips")
    set(GQLITE_CPU_ARCHITECHURE mips64)
  else()
    set(GQLITE_CPU_ARCHITECHURE x86_64)
  endif()
endif()

if(MSVC)
  if(GQLITE_CPU_ARCHITECHURE STREQUAL arm64 OR GQLITE_CPU_ARCHITECHURE STREQUAL arm)
    set(GQLITE_ASSEMBLER armasm)
  else()
    set(GQLITE_ASSEMBLER masm)
  endif()
else()
  set(GQLITE_ASSEMBLER gas)
endif()

if(GQLITE_ASM_BINFMT STREQUAL pe)
  set(GQLITE_ASM_EXT .asm)
elseif(GQLITE_ASSEMBLER STREQUAL gas)
  set(GQLITE_ASM_EXT .S)
else()
  set(GQLITE_ASM_EXT .asm)
endif()

set(GQLITE_ASM_SUFFUX ${GQLITE_CPU_ARCHITECHURE}_${GQLITE_ASM_ABI}_${GQLITE_ASM_BINFMT}_${GQLITE_ASSEMBLER}${GQLITE_ASM_EXT})

set(CMAKE_CXX_STANDARD 14)
set(CXX_14_LINK_OPTION stdc++fs)
IF (WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
enable_language(ASM_MASM)
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
enable_language(ASM)
# if (CMAKE_SYSTEM_NAME MATCHES "Android")
# add_definitions(-DANDROID_ARM_NEON=TRUE)
# else ()
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mfma")
# endif()
ELSEIF (UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic -ftree-vectorize")
set(CMAKE_C_FLAGS "-Wno-writable-strings -Wno-comment")
enable_language(ASM)

if (GQLITE_BUILD_TEST)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -fno-omit-frame-pointer -fno-optimize-sibling-calls")
endif ()
# ANDDOID
if (CMAKE_SYSTEM_NAME MATCHES "Android")
	add_definitions(-DANDROID_ARM_NEON=TRUE)
	if (ANDROID_ABI MATCHES "armeabi-v7a")
		add_definitions(-D__ARM_ARCH_2__)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon -marm")
	elseif (ANDROID_ABI MATCHES "arm64-v8a")
	elseif(ANDROID_ABI MATCHES "x86_64")
		add_definitions(-D__x86_64__)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pie -std=c99")
	elseif(ANDROID_ABI MATCHES "x86")
		add_definitions(-D__x86_64__)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pie -std=c99")

	endif()
else ()
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lrt -march=native -mfma -mavx -fopenmp")
endif()

if (CMAKE_COMPILER_IS_GNUCC)
	execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
					OUTPUT_VARIABLE GCC_VERSION)
	string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${GCC_VERSION})
	list(GET GCC_VERSION_COMPONENTS 0 GCC_MAJOR)
	# list(GET GCC_VERSION_COMPONENTS 1 GCC_MINOR)
endif()

if(GCC_MAJOR VERSION_GREATER 6.0)
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
				"test/gramma.cpp"
				"third_party/catch2/catch.hpp"
				"include/base/parallel/pam/*"
				"include/base/parallel/parlay/*"
		)
	endif()
else()
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

include_directories(./include ./third_party/libmdbx ./third_party/eigen ./third_party/fmt/include)

file(GLOB_RECURSE BASE_SOURCES "src/*.cpp")

message("asm: *_" ${GQLITE_ASM_SUFFUX})

set(ASM_CONTEXT_SOURCES "src/base/system/fcontext/jump_${GQLITE_ASM_SUFFUX}"
						"src/base/system/fcontext/make_${GQLITE_ASM_SUFFUX}"
)


# set(FMT_CAN_MODULE ON CACHE BOOL "Build a module instead of a traditional library." FORCE)
add_subdirectory(third_party/libmdbx)
add_subdirectory(third_party/fmt)

if (GQLITE_BUILD_TEST)
if (CMAKE_SYSTEM_NAME MATCHES "Android")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funwind-tables -ffunction-sections")
endif(CMAKE_SYSTEM_NAME MATCHES "Android")
enable_testing()
add_subdirectory(tool)
add_subdirectory(test)
add_subdirectory(example)
else ()
add_subdirectory(tool)
endif()

# message("USE COMPILER: " ${CMAKE_CXX_COMPILER_ID})
add_definitions(-DYY_NO_UNISTD_H)
option(GQLITE_BUILD_SHARED    "build gqlite shared library"   OFF)
if (GQLITE_BUILD_SHARED)
add_library(gqlite SHARED 
	${BASE_SOURCES} ${ASM_CONTEXT_SOURCES}
)
else()
add_library(gqlite
	${BASE_SOURCES} ${ASM_CONTEXT_SOURCES}
)
endif()
target_link_libraries(gqlite mdbx-static fmt::fmt)
target_compile_definitions(gqlite PUBLIC BOOST_CONTEXT_EXPORT=)

if (WIN32)
  
else (WIN32)
	if (GQLITE_BUILD_TEST)
		target_link_libraries(gqlite ${CMAKE_DL_LIBS} m)
	endif(GQLITE_BUILD_TEST)
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
