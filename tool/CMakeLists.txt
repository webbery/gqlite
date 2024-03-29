cmake_minimum_required(VERSION 3.0)

include_directories(../include)

add_custom_target(
  generated_tokens flex -s -o ${CMAKE_SOURCE_DIR}/include/token.cpp --header-file=${CMAKE_SOURCE_DIR}/include/token.h ${CMAKE_SOURCE_DIR}/src/gql.l
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tool
  COMMENT "Generating token.cpp ..."
)

add_custom_target(
  # generated_grammar bison -o ${CMAKE_SOURCE_DIR}/include/gql.cpp ${CMAKE_SOURCE_DIR}/src/gql.y
  generated_grammar bison -t -o ${CMAKE_SOURCE_DIR}/include/gql.cpp ${CMAKE_SOURCE_DIR}/src/gql.y
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tool
  COMMENT "Generating gql.cpp ..."
  DEPENDS generated_tokens
)

set(COMMANDLINE_SOURCE ./gqlcmd.cpp ./wcwidth.cpp ./ConvertUTF.cpp ./linenoise.cpp ${CMAKE_SOURCE_DIR}/src/base/Debug.cpp)
set(EXPORT_SOURCE ./gqlexport.cpp)
IF (WIN32)
set(L2GQL_SOURCE ./l2lite.cpp ./converter/ngql.cpp ./getopt.c)
ELSE()
set(L2GQL_SOURCE ./l2lite.cpp ./converter/ngql.cpp)
ENDIF()
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_executable(gql ${COMMANDLINE_SOURCE})
add_executable(gqldump ${EXPORT_SOURCE})
add_executable(l2lite ${L2GQL_SOURCE})
target_link_libraries(gql PUBLIC gqlite ${CXX_14_LINK_OPTION})
target_link_libraries(gqldump PUBLIC gqlite fmt-header-only ${CXX_14_LINK_OPTION})

add_test(NAME example_basketballplayer_prepare COMMAND l2lite -fngql -s../data/basketballplayer-2.X.ngql -o../test/basketballplayer-2.X/basketballplayer-2.X.gql
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# add_custom_target(
#   strip_binary strip -s -R .comment -R .gnu.version --strip-unneeded gql
#   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tool
#   COMMENT "strip gql ..."
#   DEPENDS gql
# )
install(TARGETS gql DESTINATION bin)
