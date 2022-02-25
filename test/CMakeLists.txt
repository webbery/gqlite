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

add_executable(grammar ${TEST_GRAMMAR_SOURCE})
add_executable(storage ${STORAGE_SOURCE})

target_link_libraries(grammar PUBLIC gqlite)
target_link_libraries(storage PUBLIC mdbx-static)

find_package (Python3 COMPONENTS Interpreter Development)

add_custom_target(
 before_test ALL
 COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../example/gen_scripts.py ${CMAKE_CURRENT_SOURCE_DIR}/../example
 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../example
 COMMENT "Generating test before"
)
add_dependencies(storage before_test)

add_test(NAME storage COMMAND storage)
add_test(NAME grammar COMMAND grammar)