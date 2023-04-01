cmake_minimum_required(VERSION 3.0)
project(GQLlite_Example)

set(EXAMPLE_SOURCE ./main.c)

add_executable(example ${EXAMPLE_SOURCE})

IF (UNIX)
target_link_libraries(example PUBLIC gqlite ${CXX_14_LINK_OPTION})
ELSE ()
target_link_libraries(example PUBLIC gqlite)
ENDIF()

add_test(NAME example COMMAND example)
