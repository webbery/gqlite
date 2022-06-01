cmake_minimum_required(VERSION 3.0)
project(GQLlite_Test)

include_directories(../include ../third_party/catch2)

add_definitions(-DCATCH_CONFIG_MAIN -DCATCH_CONFIG_ENABLE_BENCHMARKING)
add_definitions(-D_WORKING_DIR_=\"${CMAKE_SOURCE_DIR}\")

set(TEST_GRAMMAR_SOURCE ./grammar.cpp)
set(STORAGE_SOURCE
	./storage.cpp
	../src/StorageEngine/StorageEngine.cpp
	../src/StorageEngine/Graph.cpp
	../src/Type/Hash.cpp
	../src/Type/Binary.cpp
	../src/Type/VertexStmt.cpp
	../src/Feature/Gist.cpp
	../src/Feature/Feature.cpp
	../src/base/VertexVisitor.cc
	../src/base/ast.cc
	../src/Type/QueryCondition.cpp
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
	../src/Platform.cpp
	../src/operand/MatchPattern.cpp
	../src/operand/SimilarityOp.cpp
	../src/operand/GraphType.cpp
	../src/StorageEngine/SubGraph.cpp
	../src/Type/Vertex.cpp
	../src/Type/Edge.cpp
	../src/Graph/BipartiteGraph.cpp
	../src/operand/distance/EditDistance.cpp
	../src/operand/algorithms/Hungarian.cpp
	../src/operand/algorithms/RandomWalk.cpp
	../src/operand/match/CTreeStrategy.cpp
	../src/operand/analysis/DegreeCentrality.cpp
)

set(QUERY_SOURCE ./query.cpp
	../src/operand/query/HNSW.cpp
)

add_executable(grammar ${TEST_GRAMMAR_SOURCE})
add_executable(storage ${STORAGE_SOURCE})
add_executable(regress ${REGRESS_SOURCE})
add_executable(algorithm ${OLGRITHM_SOURCE})
IF (WIN32)
add_executable(query ${QUERY_SOURCE})
ENDIF (WIN32)

set_property(TARGET algorithm PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test")
target_link_libraries(grammar PUBLIC gqlite fmt-header-only)
target_link_libraries(regress PUBLIC gqlite fmt-header-only)
target_link_libraries(storage PUBLIC mdbx-static fmt-header-only)
target_link_libraries(algorithm fmt-header-only)
IF (WIN32)
target_link_libraries(query fmt-header-only)
ENDIF (WIN32)

# find_package(Python3 COMPONENTS Interpreter Development)

# add_custom_target(
#  before_test ALL
#  COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../example/gen_scripts.py ${CMAKE_CURRENT_SOURCE_DIR}/../example
#  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../example
#  COMMENT "Generating test before"
# )
# add_dependencies(storage before_test)
add_test(NAME storage COMMAND storage)
add_test(NAME algorithm COMMAND algorithm)
IF (WIN32)
add_test(NAME query COMMAND query)
ENDIF (WIN32)
add_test(NAME grammar COMMAND grammar)
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