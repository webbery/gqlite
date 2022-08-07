cmake_minimum_required(VERSION 3.0)
project(GQLlite_Test)

include_directories(../include ../third_party/catch2)

add_definitions(-DCATCH_CONFIG_MAIN -DCATCH_CONFIG_ENABLE_BENCHMARKING)
add_definitions(-D_WORKING_DIR_=\"${CMAKE_SOURCE_DIR}\")

set(TEST_GRAMMAR_SOURCE ./grammar.cpp)
set(STORAGE_SOURCE
	./storage.cpp
	../src/StorageEngine/StorageEngine.cpp
	)
set(PARSER_SOURCE
	../src/StorageEngine/StorageEngine.cpp
	../src/VirtualEngine.cpp
	./parser.cpp)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
set(OPTION_SOURCE ../tool/getopt.c)
else()
set(OPTION_SOURCE)
endif(CMAKE_SYSTEM_NAME MATCHES "Windows")

set(REGRESS_SOURCE
	${OPTION_SOURCE}
	./regress.cpp)

set(OLGRITHM_SOURCE ./algorithm.cpp
	../src/base/system/Platform.cpp
	../src/operand/MatchPattern.cpp
	../src/operand/SimilarityOp.cpp
	../src/operand/GraphType.cpp
	../src/StorageEngine/SubGraph.cpp
	../src/Type/Edge.cpp
	../src/Graph/BipartiteGraph.cpp
	../src/operand/distance/EditDistance.cpp
	../src/operand/algorithms/Hungarian.cpp
	../src/operand/match/CTreeStrategy.cpp
	../src/operand/analysis/DegreeCentrality.cpp
)

set(QUERY_SOURCE ./query.cpp
	../src/operand/query/HNSW.cpp
	../src/operand/query/HNSW/space_l2.cpp
	../src/operand/query/HNSW/space_ip.cpp
	../src/gutil.cpp
	../src/base/system/Platform.cpp
)

set(PTREE_SOURCE ./ptree.cpp)
set(VNET_SOURCE ./vnet.cpp
	../src/walk/WalkFactory.cpp
	../src/walk/RandomWalk.cpp
	../src/base/system/EventEmitter.cpp
	../src/Graph/EntityNode.cpp
	../src/Graph/EntityEdge.cpp
	../src/VirtualNetwork.cpp)

set(MOVIELENS_SOURCE ./movielens.cpp)

add_executable(grammar ${TEST_GRAMMAR_SOURCE})
add_compile_definitions(BINARY_DIR="${zstd_BINARY_DIR}/lib")
add_executable(storage ${STORAGE_SOURCE})
add_executable(regress ${REGRESS_SOURCE})
# add_executable(algorithm ${OLGRITHM_SOURCE})
add_executable(query ${QUERY_SOURCE})
add_executable(ptree ${PTREE_SOURCE})
add_executable(vnet ${VNET_SOURCE})
add_executable(movielens ${MOVIELENS_SOURCE})

set(storage_LINK_OPTIONS fmt::fmt-header-only mdbx-static)
set(vnet_LINK_OPTIONS fmt::fmt-header-only)
set(grammar_LINK_OPTIONS gqlite fmt::fmt-header-only)
set(movielens_LINK_OPTIONS gqlite)
set(regress_LINK_OPTIONS gqlite fmt::fmt-header-only)
IF (UNIX AND NOT APPLE)
set(TARGETS_OF_LEAK_TO_CHECK storage vnet movielens)

foreach(LEAK_TARGET ${TARGETS_OF_LEAK_TO_CHECK})
	target_compile_options(${LEAK_TARGET} PRIVATE -fsanitize=leak -fno-omit-frame-pointer)
	target_link_options(${LEAK_TARGET} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
	set(${LEAK_TARGET}_LINK_OPTIONS ${${LEAK_TARGET}_LINK_OPTIONS} asan)
endforeach()
ENDIF ()

# set_property(TARGET algorithm PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test")
target_link_libraries(grammar PUBLIC ${grammar_LINK_OPTIONS})
target_link_libraries(regress PUBLIC ${regress_LINK_OPTIONS})
add_dependencies(storage zstd)
IF (UNIX)
link_directories(${zstd_BINARY_DIR}/lib)
target_link_libraries(storage PUBLIC ${storage_LINK_OPTIONS} ${zstd_BINARY_DIR}/lib/libzstd.a)
ELSE()
target_link_libraries(storage PUBLIC ${storage_LINK_OPTIONS})
ENDIF()
# target_link_libraries(algorithm fmt::fmt-header-only)
target_link_libraries(query fmt::fmt-header-only)
target_link_libraries(movielens PUBLIC ${movielens_LINK_OPTIONS})
IF (WIN32)
target_link_libraries(vnet PRIVATE ${vnet_LINK_OPTIONS})
ELSE ()
target_link_libraries(vnet ${vnet_LINK_OPTIONS} pthread)
ENDIF ()

# find_package(Python3 COMPONENTS Interpreter Development)

# add_custom_target(
#  before_test ALL
#  COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../example/gen_scripts.py ${CMAKE_CURRENT_SOURCE_DIR}/../example
#  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../example
#  COMMENT "Generating test before"
# )
# add_dependencies(storage before_test)
add_test(NAME storage COMMAND storage)
# add_test(NAME algorithm COMMAND algorithm)
add_test(NAME query COMMAND query)
add_test(NAME grammar COMMAND grammar)
add_test(NAME ptree COMMAND ptree)
add_test(NAME vnet COMMAND vnet)
add_test(NAME regress_vertex COMMAND regress --inputdir=./vertex --outputdir=./vertex
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME regress_edge COMMAND regress --inputdir=./edge --outputdir=./edge
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME regress_query COMMAND regress --inputdir=./query --outputdir=./query
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME regress_walk COMMAND regress --inputdir=./walk --outputdir=./walk
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME regress_util COMMAND regress --inputdir=./util --outputdir=./util
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME regress_error COMMAND regress --inputdir=./error --outputdir=./error
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME movielens COMMAND movielens)

