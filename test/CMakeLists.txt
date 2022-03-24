cmake_minimum_required(VERSION 3.0)
project(GQLlite_Test)

include_directories(../include)

set(TEST_GRAMMAR_SOURCE ./grammar.cpp)
set(STORAGE_SOURCE
	./storage.cpp
	../src/StorageEngine/StorageEngine.cpp
	../src/StorageEngine/Graph.cpp
	../src/Type/Hash.cpp
	../src/Type/Binary.cpp
	../src/Type/Vertex.cpp
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

add_executable(grammar ${TEST_GRAMMAR_SOURCE})
add_executable(storage ${STORAGE_SOURCE})
add_executable(regress ${REGRESS_SOURCE})

target_link_libraries(grammar PUBLIC gqlite fmt-header-only)
target_link_libraries(regress PUBLIC gqlite fmt-header-only)
target_link_libraries(storage PUBLIC mdbx-static fmt-header-only)

# find_package(Python3 COMPONENTS Interpreter Development)

# add_custom_target(
#  before_test ALL
#  COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../example/gen_scripts.py ${CMAKE_CURRENT_SOURCE_DIR}/../example
#  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../example
#  COMMENT "Generating test before"
# )
# add_dependencies(storage before_test)

add_test(NAME storage COMMAND storage)
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