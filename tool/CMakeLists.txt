cmake_minimum_required(VERSION 3.0)
project(GQLlite_Tool)

set(CMAKE_CXX_STANDARD 14)

include_directories(../include)

message("BUILD TYPE: "${CMAKE_BUILD_TYPE}"\tBIN DIR: "${CMAKE_CURRENT_BINARY_DIR}"\tSOURCE_DIR: "${CMAKE_CURRENT_SOURCE_DIR}"\tCMAKE_SOURCE_DIR: "${CMAKE_SOURCE_DIR})

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
