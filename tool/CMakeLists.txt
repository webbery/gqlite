cmake_minimum_required(VERSION 3.0)
project(GQLlite_Tool)

set(CMAKE_CXX_STANDARD 14)

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

add_executable(gql ${COMMANDLINE_SOURCE})
add_executable(gqldump ${EXPORT_SOURCE})
add_executable(l2lite ${L2GQL_SOURCE})
target_link_libraries(gql PUBLIC gqlite fmt-header-only)
target_link_libraries(gqldump PUBLIC gqlite fmt-header-only)