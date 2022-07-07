#include "../include/gqlite.h"
#include <stdio.h>
#include "../tool/stdout.h"

#define NORMAL "\033[m"
#define RED "\033[0;32;31m"

int test_id = 0;
size_t result_count = 0;
#define TEST_GRAMMAR(nogql) {\
    ptr = nullptr;\
    printf(NORMAL"Test [%d]:\t%s\n", ++test_id, nogql); \
    if (gqlite_exec(pHandle, nogql, gqlite_exec_callback, nullptr, &ptr)) {\
      printf(RED"exec error: %s\n", ptr);\
    }\
    if (ptr) gqlite_free(ptr);\
}
#define TEST_COMMAND(cmd) {\
    ptr = nullptr;\
    printf(NORMAL"Test [%d]:\t%s\n", ++test_id, cmd); \
    if (gqlite_exec(pHandle, cmd, gqlite_cmd_callback, nullptr, &ptr)) {\
      printf(RED"exec error: %s\n", ptr);\
    }\
    if (ptr) gqlite_free(ptr);\
}

#define TEST_BOOL(nogql, status)
#define TEST_QUERY(nogql, count) \
{\
  ptr = nullptr;\
  printf(NORMAL"Test [%d]:\t%s\n", ++test_id, nogql);\
  result_count = count;\
  if (gqlite_exec(pHandle, nogql, gqlite_exec_assert_callback, nullptr, &ptr)) {\
    printf(RED"exec error: %s\n" NORMAL, ptr);\
  }\
  if (ptr) gqlite_free(ptr);\
}

int gqlite_exec_assert_callback(gqlite_result* params)
{
  if (params) {
    if (result_count != params->count) {
      printf(RED"expect result count: %d, but recieved count: %d\n" NORMAL, result_count, params->count);
    }
  }
  return 0;
}

void wrong_grammar_test(gqlite* pHandle, char* ptr) {
  TEST_GRAMMAR("{create: 'ga', noindex: 'keyword'}");
  TEST_GRAMMAR("{create: 'ga', index: b64'keyword'}");
  TEST_GRAMMAR("{create: b64'ga', index: 'keyword'}");
  TEST_QUERY("{query: '*', in: 'ga', where: {create_time: {$gt: 1.2}}}", 2);
  TEST_QUERY("{query: '*', in: {query: '*', in: 'ga', where: {create_time: {$gt: 1}}}}", 2);
  TEST_GRAMMAR("{drop: 'ga'}");
}

void successful_test(gqlite* pHandle, char* ptr) {
  // TEST_GRAMMAR("ast {create: 'ga', index: ['keyword', 'label']}");
  TEST_GRAMMAR("ast {upset: 'edge_test', edge: [ ['v1', --, 'v2'], ['v2', ->, 'v3'], ['v3', ->, 'v3'], ['v4'] ] }");
  // TEST_GRAMMAR("ast {upset: 'edge_test', vertex: [['v4', {color: '#000000', location: [131.24194, inf], create_time: 1}]]}");
  TEST_GRAMMAR("// {drop: 'ga'}");
  TEST_COMMAND("show graph");
  TEST_GRAMMAR("{drop: 'ga'}");
  TEST_COMMAND("show graph");
  //TEST_GRAMMAR("{class: 'a/c', location: [131.24194, 37.12532], keyword: ['a', 'b']}");
  /*
  * create a `ga` graph for keyword search
  */
  TEST_GRAMMAR("{create: 'ga', index: 'keyword'}");
  TEST_COMMAND("show graph");
  TEST_GRAMMAR("{create: 'ga', index: ['keyword', 'color', 'create_time']}");
  TEST_COMMAND("show graph 'ga'");
  TEST_GRAMMAR("{upset: 'ga', vertex: [['v1']]}");
  TEST_QUERY("{query: vertex, in: 'ga'}", 1);
  //printf(NORMAL"Test [%d]:\t%s\n", ++test_id, "{query: vertex, in: 'ga'}");
  //  if (gqlite_exec(pHandle, "{query: vertex, in: 'ga'}", gqlite_exec_assert_callback, nullptr, &ptr)) {
  //      printf(RED"exec error: %s\n", ptr); 
  //  }
  //gqlite_free(ptr); 
  TEST_GRAMMAR(
    "{"
      "upset: 'ga',"
      "vertex: ["
        "['v1', {class: 'a/c', location: [131.24194, 37.12532], keyword: ['a', 'b'], create_time: 1}],"
        "['v2', {color: '#343e58', keyword: ['a'], create_time: 2}],"
        "['v3', {text: b64'5Zyo57q/57yW56CB6Kej56CB', keyword: ['a'], create_time: 3}],"
        "['v4'],"
        "['v5', {keyword: [], create_time: 1}]"
    "]"
    "}");
  TEST_QUERY("{query: vertex, in: 'ga'}", 4);
  TEST_GRAMMAR("{remove: 'ga', vertex: ['v2']}");
  //printf(NORMAL"Test [%d]:\t%s\n", ++test_id, "{remove: 'ga', vertex: ['v2']}");
  //ptr = nullptr;
  //  if (gqlite_exec(pHandle, "{remove: 'ga', vertex: ['v2']}", gqlite_exec_callback, nullptr, &ptr)) {
  //      printf(RED"exec error: %s\n", ptr);
  //  }
  //if (ptr) gqlite_free(ptr);
  TEST_QUERY("{query: vertex, in: 'ga'}", 3);
  TEST_QUERY("{query: vertex, in: 'ga', where: {create_time: {$gt: 1, $lt: 5}}}", 1);
  TEST_QUERY("{query: vertex, in: 'ga', where: {create_time: {$gte: 1, $lt: 5}}}", 2);
  TEST_QUERY("{query: vertex, in: 'ga', where: {id: 'v1'}}", 1);
  TEST_QUERY("{query: vertex, in: 'ga', where: {keyword: 'b'}}", 1);
  TEST_QUERY("{query: vertex, in: 'ga', where: {create_time: {$gt: 1}}}", 1);
  TEST_QUERY("{query: vertex, in: 'ga', where: {create_time: {$lt: 5}}}", 2);
  TEST_GRAMMAR("{query: [vertex.class], in: 'ga', where: {keyword: 'b'}}");
  TEST_GRAMMAR("{query: [vertex.class], in: 'ga', where: {keyword: 'b'}}");
  // TEST_GRAMMAR("{dump: 'nogql.gql'}");
  /*
  * EDGES & LINKS
  */
  TEST_GRAMMAR(
    "{"
      "upset: 'ga',"
      "edge: ["
        "['v1', --, 'v2']"
      "]"
    "}"
  );
  TEST_GRAMMAR(
    "{"
      "upset: 'ga',"
      "edge: ["
        "['v2', ->, 'v3']"
      "]"
    "}"
  );
  TEST_GRAMMAR(
    "{"
      "upset: 'ga',"
      "edge: ["
        "['v3', ->, 'v3']"
      "]"
    "}"
  );
  // loop circle
  TEST_GRAMMAR(
    "{"
      "upset: 'ga',"
      "edge: ["
        "['v4']"
      "]"
    "}"
  );
  // query 1'st order neighber
  TEST_QUERY("{query: vertex, in: 'ga', where: {id: 'v1', ->: 1}}", 0);
  TEST_QUERY("{query: vertex, in: 'ga', where: {id: 'v1', --: 1}}", 1);
  // TEST_GRAMMAR("{create: 'prefix_tree'}");
  // TEST_GRAMMAR(
  //   "{"
  //     "upset: 'prefix_tree',"
  //     "vertex: ["
  //       "['b', {item: [['v1', {weight: 0.6}]]}]"
  //     "]"
  //   "}");
  // TEST_GRAMMAR("{upset: 'prefix_tree', edge: [['b', ->, 'e']]}");
  // TEST_GRAMMAR("{query: '*', path: ['b', 'e', ...], from: 'prefix_tree'}");
  /*
  * search item with distance
  */
  // TEST_GRAMMAR("{query: '*', from: 'ga', sort: 'distance', where: {"
  //   "distance: function (location, item) {"
  //       "return abs(location[0] - item[0]) + abs(location[1] - item[1]);"
  //     "}"
  //   "}}");
  /*
  * search path from a to b
  */
  TEST_GRAMMAR("{query: path, in: 'ga', from: 'v1', to: 'v2', where: {--: 'dijk'}}");
  TEST_QUERY("{query: edge.type('*'), in: 'ga', where: {id: 'v1', --: 1}}", 1);
  // TEST_GRAMMAR("{walk: 'biBFS', in: 'ga', from: 'a', to: 'b'}");
  // TEST_GRAMMAR("{walk: 'AStar', in: 'ga', from: 'a', to: 'b', cost: () => {return random()}}");

  // search 1'st neighbor of item
  // TEST_GRAMMAR("{query: '*', from: 'ga', where: {neighbor: 1}");

  TEST_GRAMMAR("{drop: 'ga'}");
  // TEST_GRAMMAR("dump {query: vertex, in: 'ga', where: {id: 'v1', --: 1}}");
}

void test_edges() {}

int main() {
    gqlite* pHandle = 0;
    gqlite_open(&pHandle);
    char* ptr = nullptr;
    successful_test(pHandle, ptr);
    wrong_grammar_test(pHandle, ptr);
    gqlite_close(pHandle);
    return 0;
}