#include "../include/gqlite.h"
#include <stdio.h>
#include "../tool/stdout.h"

#define NORMAL "\033[m"
#define RED "\033[0;32;31m"

int test_id = 0;
size_t current_count = 0;
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
  current_count = 0;\
  if (gqlite_exec(pHandle, nogql, gqlite_exec_assert_callback, nullptr, &ptr)) {\
    printf(RED"exec error: %s\n" NORMAL, ptr);\
  }\
  if (current_count != count) {\
    printf(RED"expect result count: %d, but recieved count: %d\n" NORMAL, count, current_count);\
  }\
  if (ptr) gqlite_free(ptr);\
}

int gqlite_exec_assert_callback(gqlite_result* params, void*)
{
  if (params) {
    current_count += params->count;
  }
  return 0;
}

void wrong_grammar_test(gqlite* pHandle, char* ptr) {
  TEST_GRAMMAR("{create: 'ga', noindex: 'keyword'};");
  TEST_GRAMMAR("{create: 'ga', index: b64'keyword'};");
  TEST_GRAMMAR("{create: b64'ga', index: 'keyword'};");
  TEST_QUERY("{query: '*', in: 'ga', where: {create_time: {$gt: 1.2}}};", 2);
  TEST_QUERY("{query: '*', in: {query: '*', in: 'ga', where: {create_time: {$gt: 1}}}};", 2);
  TEST_QUERY("{query: 'g', in: 'ga', where: {$and: [create_time: {$lt: 5}]}};", 3);
  TEST_GRAMMAR("{drop: 'ga'};");
}

void successful_test(gqlite* pHandle, char* ptr) {
  // TEST_GRAMMAR("ast {create: 'ga', index: ['keyword', 'label']}");
  TEST_GRAMMAR("ast {upset: 'edge_test', edge: [ ['v1', --, 'v2'], ['v2', ->, 'v3'], ['v3', ->, 'v3'], ['v4'] ] };");
  // TEST_GRAMMAR("ast {upset: 'edge_test', vertex: [['v4', {color: '#000000', location: [131.24194, inf], create_time: 1}]]}");
  TEST_GRAMMAR("// {drop: 'ga'};");
  TEST_COMMAND("show graph;");
  TEST_GRAMMAR("{drop: 'ga'};");
  TEST_COMMAND("show graph;");
  //TEST_GRAMMAR("{class: 'a/c', location: [131.24194, 37.12532], keyword: ['a', 'b']}");
  /*
  * create a `ga` graph for keyword search
  */
  TEST_GRAMMAR("{create: 'ga', group: ['g']};");
  TEST_COMMAND("show graph;");
  TEST_GRAMMAR("{create: 'ga', group: ['g', 'e', 'tag']};");
  TEST_GRAMMAR("{create: 'ga', group: [{g: ['title', 'class', 'keyword', 'color', 'create_time', 'location'], index: ['keyword', 'color', 'create_time', 'location']}, 'e', 'tag']};");
  TEST_COMMAND("show graph 'ga';");
  TEST_GRAMMAR("{upset: 'g', vertex: [[328, {title: 'Tale\\'s from the Crypt Presents: Demon Knight (1995)', genres: 'Horror|Thriller'}]]};");
  TEST_GRAMMAR("{upset: 'g', vertex: [['328', {title: 'Tale\\'s from the Crypt Presents: Demon Knight (1995)', genres: 'Horror|Thriller'}]]};");
  TEST_QUERY("{query: 'g'};", 1);
  TEST_QUERY("{query: 'g', in: 'ga'};", 1);
  TEST_GRAMMAR("{upset: 'g', vertex: [[55, {update_time: 0d12345}]]};");
  TEST_GRAMMAR("{upset: 'g', vertex: [[1040187648, {datetime:'0d1600577894000',filename: 'f2d470a08a1011eab5a4993e17631b31.jpg~tplv-banciyuan-w650.jpg',hash: 'unknow',height: 650,path: 'C:\\Users\\webberg\\Pictures\\f2d470a08a1011eab5a4993e17631b31.jpg~tplv-banciyuan-w650.jpg',size: 207879,type: 'unknow',width: 650}]]};");
  TEST_QUERY("{query: 'g', in: 'ga', where: {datetime: {$gt:0d1600531200000}}};", 1);
  TEST_GRAMMAR("{upset: 'g', property: {filename: '7777.jpg'}, where: {id: 1040187648}};");
  TEST_GRAMMAR(
    "{"
      "upset: 'g',"
      "vertex: ["
        "[42, {class: 'a/c', location: [131.24194, 37.12532], keyword: ['a', 'b'], create_time: 1}],"
        "[46, {color: [52, 62, 88], location: [131.24194, 36.12532], keyword: ['a'], create_time: 2, update_time: 0d1664268855}],"
        "[1, {text: 0b'5Zyo57q/57yW56CB6Kej56CB', location: [131.25194, 32.11532], keyword: ['a'], create_time: 145377}],"
        "[45],"
        "[4, {keyword: [], create_time: 1}]"
    "]"
    "};");
  TEST_GRAMMAR("{upset: 'g', vertex: [[456, {name:'新分类2/新子类', pid:2821611776}]]};");
  TEST_QUERY("{query: 'g', in: 'ga'};", 7);
  TEST_QUERY("{query: 'g', in: 'ga', where: {pid: 461791488}};", 1);
  TEST_GRAMMAR("{remove: 'g', vertex: ['1']};");
  TEST_QUERY("{query: 'g', in: 'ga'};", 6);
  TEST_QUERY("{query: 'g', in: 'ga', where: {update_time: {$lt: 0d1653315732}}};", 1);
  TEST_QUERY("{query: 'g', in: 'ga', where: {create_time: {$gt: 1, $lt: 5}}};", 1);
  TEST_QUERY("{query: 'g', in: 'ga', where: {create_time: {$gte: 1, $lt: 5}}};", 3);
  TEST_QUERY("{query: 'g', in: 'ga', where: {id: 'v1'}};", 0);
  TEST_QUERY("{query: 'g', in: 'ga', where: {keyword: 'b'}};", 1);
  TEST_QUERY("{query: 'g', in: 'ga', where: {create_time: {$gt: 1}}};", 1);
  TEST_QUERY("{query: 'g', in: 'ga', where: {create_time: {$lt: 5}}};", 3);
  TEST_QUERY("{query: 'g', in: 'ga', where: {$and: [{create_time: {$lt: 5}}]}};", 3);
  TEST_QUERY("{query: 'g', in: 'ga', where: {$or: [{create_time: {$lt: 5}}]}};", 3);
  TEST_GRAMMAR(
    "{"
      "query: 'g', in: 'ga',"
      "where: {"
        "color: {"
          "$near: {"
            "$geometry: [126, 126, 126],"
            "$lt: 10"
          "}"
        "}"
      "}"
    "};");
  TEST_GRAMMAR(
    "{"
      "query: 'g', in: 'ga',"
      "where: {"
        "color: {"
          "$lt: ()=>{(1+2)*2/3-1;return 10;}"
          //"$lt: ()=>{return 0.5;}"
        "}"
      "}"
    "};");
  TEST_GRAMMAR("{query: 'g', in: 'ga'};");
  TEST_GRAMMAR("{query: [g.class], in: 'ga', where: {keyword: 'b'}};");
  TEST_GRAMMAR("{query: [g.class], in: 'ga', where: {keyword: 'b'}};");
  TEST_GRAMMAR("{dump: 'ga'};");
  /*
  * EDGES & LINKS
  */
 
  TEST_GRAMMAR("{upset: 'tag', edge: [[474, --: {tag: 'robots'}, 589]]};");
  TEST_GRAMMAR(
    "{upset: 'e', edge: ["
      "['player149', ->: {start_year: 2016, end_year: 2019 }, 'team219']"
    "]};");
  TEST_GRAMMAR(
    "{"
      "upset: 'e',"
      "edge: ["
        "['v1', --, 'v2']"
      "]"
    "};"
  );
  TEST_GRAMMAR(
    "{"
      "upset: 'e',"
      "edge: ["
        "['v2', ->, 'v3']"
      "]"
    "};"
  );
  // loop circle
  TEST_GRAMMAR(
    "{"
      "upset: 'e',"
      "edge: ["
        "['v4']"
      "]"
    "};"
  );
  TEST_GRAMMAR(
    "{"
      "upset: 'e',"
      "edge: ["
        "['v3', ->, 'v3']"
      "]"
    "};"
  );
  // query 1'st order neighber
  TEST_QUERY("{query: 'e', in: 'ga'};", 4);
  TEST_QUERY("{query: 'e', in: 'ga', where: ['v1', --, *]};", 1);
  TEST_QUERY("{query: 'e', in: 'ga', where: ['v1', ->, *]};", 0);
  TEST_QUERY("{query: 'e', in: 'ga', where: {id: 'v1', ->: *, neighbor: 1}};", 0);
  TEST_QUERY("{query: 'e', in: 'ga', where: {id: 'v1', --: *, neighbor: 1}};", 1);
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
  //TEST_GRAMMAR("{query: path, in: 'ga', from: 'v1', to: 'v2', where: {--: 'dijk'}};");
  // TEST_QUERY("{query: edge.type('*'), in: 'ga', where: {id: 'v1', --: 1}}", 1);
  // TEST_GRAMMAR("{walk: 'biBFS', in: 'ga', from: 'a', to: 'b'}");
  // TEST_GRAMMAR("{walk: 'AStar', in: 'ga', from: 'a', to: 'b', cost: () => {return random()}}");

  // search 1'st neighbor of item
  // TEST_GRAMMAR("{query: '*', from: 'ga', where: {neighbor: 1}");

  TEST_GRAMMAR("{drop: 'ga'};");
  // TEST_GRAMMAR("dump {query: vertex, in: 'ga', where: {id: 'v1', --: 1}}");
}

void test_edges() {}

int main() {
    gqlite* pHandle = 0;
    gqlite_open(&pHandle, "测试");
    char* ptr = nullptr;
    successful_test(pHandle, ptr);
    wrong_grammar_test(pHandle, ptr);
    gqlite_close(pHandle);
    return 0;
}