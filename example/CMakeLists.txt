cmake_minimum_required(VERSION 3.0)
project(GQLlite_Example)

set(EXAMPLE_SOURCE ./main.c)

add_executable(example ${EXAMPLE_SOURCE})

target_link_libraries(example PUBLIC gqlite)

add_test(NAME example COMMAND example)
