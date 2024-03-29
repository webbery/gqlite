cmake_minimum_required(VERSION 3.8)
project(GQLlite_Test)

include_directories(../include ../third_party/catch2)

add_definitions(-DCATCH_CONFIG_MAIN -DCATCH_CONFIG_ENABLE_BENCHMARKING)
add_definitions(-D_WORKING_DIR_=\"${CMAKE_SOURCE_DIR}\")

if (GQLITE_BUILD_TEST)
if (UNIX)
set(SYMBOLS_SOURCE ../src/base/Debug.cpp)
endif(UNIX)
else()
endif()

set(ASM_COROUTINE_SOURCES "../src/base/system/fcontext/jump_${GQLITE_ASM_SUFFUX}"
						"../src/base/system/fcontext/make_${GQLITE_ASM_SUFFUX}"
)

set(TEST_GRAMMAR_SOURCE ./grammar.cpp ${SYMBOLS_SOURCE})
set(STORAGE_SOURCE
	./storage.cpp
	../src/StorageEngine/StorageEngine.cpp
	../src/base/Debug.cpp
	../src/gutil.cpp
	${SYMBOLS_SOURCE}
	)
set(PARSER_SOURCE
	../src/StorageEngine/StorageEngine.cpp
	../src/VirtualEngine.cpp
	./parser.cpp
	../src/gutil.cpp
	${SYMBOLS_SOURCE}
	)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
set(OPTION_SOURCE ../tool/getopt.c)
else()
set(OPTION_SOURCE)
endif()

set(REGRESS_SOURCE
	${OPTION_SOURCE}
	./regress.cpp
	${SYMBOLS_SOURCE}
	)

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
	${SYMBOLS_SOURCE}
)

set(QUERY_SOURCE ./query.cpp
	../src/operand/query/HNSW.cpp
	../src/base/math/Distance.cpp
	../src/StorageEngine/StorageEngine.cpp
	../src/VirtualNetwork.cpp
	../src/gutil.cpp
	../src/walk/AStarWalk.cpp
	../src/walk/RandomWalk.cpp
	../src/base/system/Platform.cpp
	../src/base/system/Coroutine.cpp
	../src/schedule/DefaultSchedule.cpp
	${ASM_COROUTINE_SOURCES}
	${SYMBOLS_SOURCE}
)

set(PTREE_SOURCE ./ptree.cpp
	${SYMBOLS_SOURCE}
)
set(VNET_SOURCE ./vnet.cpp
	../src/walk/RandomWalk.cpp
	../src/walk/AStarWalk.cpp
	../src/base/system/Coroutine.cpp
	../src/Graph/EntityNode.cpp
	../src/Graph/EntityEdge.cpp
	../src/StorageEngine/StorageEngine.cpp
	../src/VirtualNetwork.cpp
	../src/gutil.cpp
	${SYMBOLS_SOURCE}
)

set(ALGRITHM_SOURCE ./algorithm.cpp
	../src/operand/algorithms/Hungarian.cpp
	${SYMBOLS_SOURCE}
)
set(MOVIELENS_SOURCE ./movielens.cpp
	${SYMBOLS_SOURCE}
)

set(DECISION_SOURCE ./decision.cpp
  ../src/Graph/DecisionNetwork.cpp
  ../src/operand/algorithms/DecisionTree.cpp
  ${SYMBOLS_SOURCE}
)

set(INFOMATION_SOURCE ./infomation.cpp
  ../src/base/math/Entropy.cpp
	${SYMBOLS_SOURCE}
)

set(MATCH_SOURCE	./match.cpp
  ../src/walk/AStarWalk.cpp
	${SYMBOLS_SOURCE}
)

set(FAIL_SOURCE ./fail.cpp
	${SYMBOLS_SOURCE}
)

set(BENCHMARK_SOURCE ./benchmark.cpp)

set(COROUTINE_SOURCE ./coro.cpp
  ../src/base/system/Coroutine.cpp
  ../src/schedule/DefaultSchedule.cpp
  ${SYMBOLS_SOURCE}
)

IF (APPLE)
ELSEIF (UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finstrument-functions")
ELSE ()
ENDIF ()

add_executable(grammar ${TEST_GRAMMAR_SOURCE})
add_executable(storage ${STORAGE_SOURCE})
add_executable(regress ${REGRESS_SOURCE})
add_executable(algorithm ${ALGRITHM_SOURCE})
add_executable(query ${QUERY_SOURCE})
add_executable(ptree ${PTREE_SOURCE})
add_executable(vnet ${VNET_SOURCE})
add_executable(movielens ${MOVIELENS_SOURCE})
add_executable(decision ${DECISION_SOURCE})
add_executable(infomation ${INFOMATION_SOURCE})
add_executable(match ${MATCH_SOURCE})
# add_executable(fail ${FAIL_SOURCE})
add_executable(benchmark ${BENCHMARK_SOURCE})
add_executable(coroutine ${ASM_COROUTINE_SOURCES} ${COROUTINE_SOURCE})
# set_source_files_properties(${ASM_COROUTINE_SOURCES} PROPERTIES COMPILE_FLAGS "/safeseh")

set(storage_LINK_OPTIONS fmt::fmt-header-only mdbx-static)
set(vnet_LINK_OPTIONS gqlite)
set(grammar_LINK_OPTIONS gqlite fmt::fmt-header-only)
set(movielens_LINK_OPTIONS gqlite)
set(regress_LINK_OPTIONS gqlite fmt::fmt-header-only)
set(benchmark_LINK_OPTIONS gqlite fmt::fmt-header-only)
# set(fail_LINK_OPTIONS gqlite)
IF (UNIX AND NOT APPLE AND NOT (CMAKE_SYSTEM_NAME MATCHES "Android"))
set(TARGETS_OF_LEAK_TO_CHECK storage movielens decision infomation decision match query)

foreach(LEAK_TARGET ${TARGETS_OF_LEAK_TO_CHECK})
	target_compile_options(${LEAK_TARGET} PRIVATE -fsanitize=leak -fno-omit-frame-pointer)
	target_link_options(${LEAK_TARGET} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
	set(${LEAK_TARGET}_LINK_OPTIONS ${${LEAK_TARGET}_LINK_OPTIONS} asan)
endforeach()
ELSE()
#add_definitions(-D_CRTDBG_MAP_ALLOC)
ENDIF ()

# set_property(TARGET algorithm PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test")
target_link_libraries(grammar PUBLIC ${grammar_LINK_OPTIONS})
target_link_libraries(regress PUBLIC ${regress_LINK_OPTIONS})
target_link_libraries(benchmark PUBLIC ${benchmark_LINK_OPTIONS})
target_link_libraries(movielens PUBLIC ${movielens_LINK_OPTIONS})
# target_link_libraries(fail PUBLIC ${fail_LINK_OPTIONS})
IF (UNIX)
target_link_libraries(storage PUBLIC ${storage_LINK_OPTIONS})
target_link_libraries(query PUBLIC ${storage_LINK_OPTIONS})
if (GQLITE_BUILD_TEST)
target_link_libraries(storage PUBLIC ${CMAKE_DL_LIBS} m ${CXX_14_LINK_OPTION})
target_link_libraries(query PUBLIC ${CMAKE_DL_LIBS} m  ${CXX_14_LINK_OPTION})
target_link_libraries(movielens PUBLIC ${CMAKE_DL_LIBS} m ${CXX_14_LINK_OPTION})
target_link_libraries(grammar PUBLIC ${CMAKE_DL_LIBS} m ${CXX_14_LINK_OPTION})
target_link_libraries(regress PUBLIC ${CMAKE_DL_LIBS} m  ${CXX_14_LINK_OPTION})
target_link_libraries(benchmark PUBLIC ${CMAKE_DL_LIBS} m  ${CXX_14_LINK_OPTION})
endif(GQLITE_BUILD_TEST)
ELSE() # UNIX
target_link_libraries(storage PUBLIC ${storage_LINK_OPTIONS})
target_link_libraries(query PUBLIC ${storage_LINK_OPTIONS})
ENDIF() # UNIX
# target_link_libraries(algorithm fmt::fmt-header-only)
IF (WIN32)
target_link_libraries(vnet PRIVATE ${vnet_LINK_OPTIONS})
ELSE (WIN32)
if (CMAKE_SYSTEM_NAME MATCHES "Android")
target_link_libraries(vnet ${vnet_LINK_OPTIONS} log)
target_link_libraries(decision log)
target_link_libraries(infomation log)
target_link_libraries(match log)
target_link_libraries(algorithm log)
# target_link_libraries(grammar log)
# target_link_libraries(ptree log)
# target_link_libraries(regress log)
# target_link_libraries(movielens log)
target_link_libraries(coroutine gqlite log)
else ()
target_link_libraries(vnet ${vnet_LINK_OPTIONS} pthread ${CXX_14_LINK_OPTION})
endif ()
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
add_test(NAME query COMMAND query)
add_test(NAME grammar COMMAND grammar)
add_test(NAME ptree COMMAND ptree)
# add_test(NAME fail COMMAND fail)
add_test(NAME decision COMMAND decision)
add_test(NAME infomation COMMAND infomation)
add_test(NAME match COMMAND match)
add_test(NAME basketball_player_prepare COMMAND regress --inputdir=./basketballplayer-2.X --outputdir=./basketballplayer-2.X --dbfile=./basketballplayer
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME regress_vertex COMMAND regress --inputdir=./vertex --outputdir=./vertex
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME regress_edge COMMAND regress --inputdir=./edge --outputdir=./edge
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME regress_query COMMAND regress --inputdir=./query --outputdir=./query
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME regress_walk COMMAND regress --inputdir=./walk --outputdir=./walk
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME regress_util COMMAND regress --inputdir=./walk --outputdir=./util --dbfile=./vertex_db
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME regress_error COMMAND regress --inputdir=./error --outputdir=./error
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME movielens COMMAND movielens)
add_test(NAME vnet COMMAND vnet)
add_test(NAME coroutine COMMAND coroutine)
add_test(NAME benchmark_basketball_player COMMAND benchmark ./benchmark/vertex.gs ./basketballplayer
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# set_tests_properties(fail PROPERTIES WILL_FAIL TRUE)
