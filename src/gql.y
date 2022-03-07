%define api.pure full
%locations
%define parse.error verbose
// %define api.prefix {gql_yy}
%param { yyscan_t scanner }
%code requires {
  typedef void* yyscan_t;
}

%code top{
#include <stdio.h>
#include <set>
#include "Error.h"
#include "gql/creation.h"
#include "gql/query.h"
#include "gql/upset.h"
#include "Type/Binary.h"
#include "base/ast.h"
#include "base/VertexVisitor.h"
#include "base/EdgeVisitor.h"
#include "Singlecton.h"

#define MAX_VARIANT_SIZE  32
#define GET_GRAPH(name)  \
  GGraph* pGraph = GSinglecton::get<GStorageEngine>()->getGraph(name);\
  if (!pGraph) {\
    break;\
  }
#define INIT_ITEM(key, value, type) \
  size_t len = strlen(key) + 1;\
  void* s = malloc(len);\
  memcpy(s, key, len);\
  struct gast* k = newast(NodeType::String, s, nullptr, nullptr);\
  struct gast* root = newast(type, nullptr, k, value)
}

%lex-param {GStatement& stm}
%parse-param {GStatement& stm}
%union {
  struct gast* __node;
  char var_name[MAX_VARIANT_SIZE];
  float __f;
  gql_node* __list;
  char* __c;
  GGraph* _g;
  size_t __offset;
  time_t __datetime;
  int32_t __int;
  nlohmann::json* __json;
}

%code {
#define YY_DECL \
       int yylex(YYSTYPE* yylval_param, YYLTYPE* yylloc_param, void* yyscanner, GStatement& stm)

void yyerror(YYLTYPE* yyllocp, yyscan_t unused, GStatement& stm, const char* msg) {
  std::string err_index;
  if (stm._errIndx) {
    std::string err_offset(stm._errIndx - 1, ' ');
    err_index += err_offset + "~";
  }
  printf("\033[22;31mError:\t%s:\033[22;0m\n\t%s\n\t%s\n",
    msg, stm.gql().c_str(), err_index.c_str());
}

void release_vertex_callback(gql::vertex*& v) {}

struct gast* INIT_STRING_AST(const char* key) {
  size_t len = strlen(key) + 1;
  void* s = malloc(len);
  memcpy(s, key, len);
  // printf("|-> %s\n", key);
  return newast(NodeType::String, s, nullptr, nullptr);
}

template<typename T>
struct gast* INIT_BASIC_TYPE_AST(T v, NodeType type) {
  T* value = (T*)malloc(sizeof(T));
  *value = v;
  return newast(type, value, nullptr, nullptr);
}

template<typename T>
struct gast* INIT_LITERAL_AST(T& v, NodeType type) {
  void* value = malloc(sizeof(T));
  memcpy(value, &v, sizeof(T));
  return newast(type, value, nullptr, nullptr);
}

void init_result_info(gqlite_result& result, const std::vector<std::string>& info) {
  result.count = info.size();
  result.infos = (char**)malloc(result.count * sizeof(char*));
  for (size_t idx = 0; idx < result.count; ++idx) {
    result.infos[idx] = (char*)malloc(info[idx].size() + 1);
    memcpy(result.infos[idx], info[idx].data(), info[idx].size() + 1);
  }
  result.type = gqlite_result_type_cmd;
}

void release_result_info(gqlite_result& result) {
  for (size_t idx = 0; idx < result.count; ++idx) {
    free(result.infos[idx]);
  }
  free(result.infos);
}

nlohmann::json* get_or_create_json(nlohmann::json* item) {
  if (item->empty()) {
    nlohmann::json* j = new nlohmann::json();
    return j;
  }
  return item;
}
}

%token RANGE_BEGIN RANGE_END COLON QUOTE COMMA LEFT_SQUARE RIGHT_SQUARE STAR CR PARAM_BEGIN PARAM_END
%token <var_name> VAR_STRING VAR_HASH
%token <__f> VAR_DECIMAL
%token <__c> VAR_BASE64
%token <__int> VAR_INTEGER
%token <__int> VAR_DATETIME
%token KW_DUMP KW_ID KW_GRAPH KW_COMMIT
%token KW_CREATE KW_DROP KW_IN KW_REMOVE KW_UPSET KW_LEFT_RELATION KW_RIGHT_RELATION KW_BIDIRECT_RELATION KW_REST KW_DELETE
%token OP_QUERY OP_FROM KW_INDEX KW_GROUP OP_WHERE OP_TO
%token CMD_SHOW 
%token OP_GREAT_THAN OP_LESS_THAN OP_GREAT_THAN_EQUAL OP_LESS_THAN_EQUAL OP_EQUAL OP_AND OP_OR
%token FN_COUNT
%token dot
%token <__node> KW_VERTEX
%token <__node> KW_EDGE
%token <__node> KW_PATH

%type <__list> vertex_list vertexes string_list strings property_list edges edge_list
%type <_g> a_graph_expr
%type <__node> json
%type <__node> value
%type <__node> values
%type <__node> object
%type <__node> array
%type <__node> properties
%type <__node> where_expr
%type <__node> function_call
%type <__node> property
%type <__node> vertex
%type <__node> edge
%type <__node> gql
%type <__node> upset_vertexes
%type <__node> upset_edges
%type <__node> a_simple_query
%type <__node> query_kind_expr
%type <__node> query_kind
%type <__node> a_graph_properties
%type <__node> a_edge
%type <__node> a_link_condition
%type <__node> a_value

%start line_list

%%
line_list: line
          | line_list line
          ;
    line: gql {}
        | utility_cmd { stm._cmdtype = GQL_Util; }
        | {}
        ;
    gql: creation  { stm._cmdtype = GQL_Creation; }
        | a_simple_query { $$ = $1; stm._cmdtype = GQL_Query; }
        | upset_vertexes { $$ = $1; stm._cmdtype = GQL_Upset;}
        | upset_edges { $$ = $1; stm._cmdtype = GQL_Upset; }
        | remove_vertexes { stm._cmdtype = GQL_Remove; }
        | drop_graph { stm._cmdtype = GQL_Drop; }
        | dump { stm._cmdtype = GQL_Util; }
        | test {}
        ;

utility_cmd: CMD_SHOW KW_GRAPH
          {
            std::vector<std::string> vg = GSinglecton::get<GStorageEngine>()->getGraphs();
            gqlite_result results;
            init_result_info(results, vg);
            stm._result_callback(&results);
            release_result_info(results);
            stm._errorCode = ECode_Success;
          }
        | CMD_SHOW KW_GRAPH VAR_STRING
          {
            GGraph* g = GSinglecton::get<GStorageEngine>()->getGraph($3);
            const GraphProperty& props = g->property();
            char buff[256] = {0};
            sprintf(buff, "show graph [%s]:\n", $3);
            std::string str(buff);
            for (auto& name: props._indexes) {
              memset(buff, 0, 256);
              sprintf(buff, "\tindex: %s\n", name.c_str());
              str += buff;
            }
            gqlite_result results;
            init_result_info(results, {str});
            stm._result_callback(&results);
            release_result_info(results);
            stm._errorCode = ECode_Success;
          }
          | KW_DUMP gql {};
creation: RANGE_BEGIN KW_CREATE VAR_STRING RANGE_END
            {
              GSinglecton::get<GStorageEngine>()->openGraph($3);
              stm._errorCode = ECode_Success;
            }
        | RANGE_BEGIN KW_CREATE VAR_STRING COMMA KW_INDEX COLON string_list RANGE_END
            {
              GSinglecton::get<GStorageEngine>()->openGraph($3);
              GGraph* g = GSinglecton::get<GStorageEngine>()->getGraph($3);
              gql_node* cur = $7;
              do {
                gast* s = (struct gast*)(cur->_value);
                std::string value = GET_STRING_VALUE(s);
                creation::createInvertIndex(g, value.c_str());
                cur = cur->_next;
              } while(cur);
              stm._errorCode = ECode_Success;
            }
        | RANGE_BEGIN KW_CREATE VAR_STRING COMMA KW_INDEX COLON function_call RANGE_END {};
dump: RANGE_BEGIN KW_DUMP COLON VAR_STRING RANGE_END
            {
              if (strlen($4) == 0) {
                break;
              }
              std::vector<std::string> gs = GSinglecton::get<GStorageEngine>()->getGraphs();
              std::string dump;
              for (std::string& gname: gs) {
                dump += creation::generateGraphScript(gname);
                GGraph* g = GSinglecton::get<GStorageEngine>()->getGraph(gname.c_str());
                if (!g) {
                  continue;
                }
                std::string vertexes = upset::exportVertexes(g);
                if (vertexes.size()) {
                  dump += vertexes;
                  dump += ",";
                }
                std::string edges = upset::exportEdges(g);
                if (edges.size()) {
                  dump += edges;
                  dump += ",";
                }
              }
              dump.pop_back();
              FILE* f = fopen($4, "w");
              if (!f) break;
              fwrite(dump.c_str(), 1, dump.size(), f);
              fclose(f);
              // gprint(LITE_DEBUG, "dump graph %s finish", $4);
              stm._errorCode = ECode_Success;
            };
upset_vertexes: RANGE_BEGIN KW_UPSET COLON VAR_STRING COMMA KW_VERTEX COLON vertex_list RANGE_END
              {
                GET_GRAPH($4);
                gql_node* cur = $8;
                ASTVertexUpdateVisitor visitor;
                while(cur) {
                  gast* pv = (gast*)(cur->_value);
                  traverse(pv, &visitor);
                  cur = cur->_next;
                }
                release_list($8, release_vertex_callback);
                GSinglecton::get<GStorageEngine>()->finishUpdate(pGraph);
                stm._errorCode = ECode_Success;
              };
remove_vertexes: RANGE_BEGIN KW_REMOVE COLON VAR_STRING COMMA KW_VERTEX COLON array RANGE_END
              {
                GET_GRAPH($4);
                gql_node* list = as_array($8);
                gql_node* current = list;
                while(current) {
                  gast* node = (struct gast*)current->_value;
                  // if (node->)
                  char* nodeid = (char*)node->_value;
                  GSinglecton::get<GStorageEngine>()->dropNode(pGraph, nodeid);
                  current = current->_next;
                }
                GSinglecton::get<GStorageEngine>()->finishUpdate(pGraph);
                stm._errorCode = ECode_Success;
              };
upset_edges: RANGE_BEGIN KW_UPSET COLON VAR_STRING COMMA KW_EDGE COLON edge_list RANGE_END
              {
                GET_GRAPH($4);
                gql_node* cur = $8;
                ASTEdgeUpdateVisitor visitor;
                while(cur) {
                  gast* pv = (gast*)(cur->_value);
                  traverse(pv, &visitor);
                  cur = cur->_next;
                }
                stm._errorCode = ECode_Success;
              };
drop_graph: RANGE_BEGIN KW_DROP COLON VAR_STRING RANGE_END
              {
                GET_GRAPH($4);
                GSinglecton::get<GStorageEngine>()->dropGraph(pGraph);
                stm._errorCode = ECode_Success;
              };
a_simple_query: 
        | RANGE_BEGIN query_kind COMMA a_graph_expr RANGE_END
                {
                  if (!$4) break;
                  std::vector<VertexID> ids = GSinglecton::get<GStorageEngine>()->getNodes($4);
                  gqlite_result results;
                  query::get_vertexes($4, ids, results);
                  query::filter_property(results, $2);
                  results.type = gqlite_result_type_node;
                  stm._result_callback(&results);
                  query::release_vertexes(results);
                  stm._errorCode = ECode_Success;
                  $$ = nullptr;
                }
        | RANGE_BEGIN query_kind COMMA a_graph_expr COMMA where_expr RANGE_END
                {
                  if (!$4) break;
                  const GraphProperty& properties = $4->property();
                  ASTVertexQueryVisitor visitor;
                  traverse($6, &visitor);
                  std::set<VertexID> sIds;
                  stm._errorCode = $4->queryVertex(sIds, visitor.conditions());
                  if(!stm._errorCode && sIds.size() == 0) break;
                  gqlite_result results;
                  results.type = gqlite_result_type_node;
                  query::get_vertexes($4, sIds, results);
                  query::filter_property(results, $2);
                  stm._result_callback(&results);
                  query::release_vertexes(results);
                };
        | RANGE_BEGIN query_kind COMMA a_graph_expr COMMA a_path_plan COMMA where_expr RANGE_END {};
query_kind: OP_QUERY COLON query_kind_expr { $$ = $3; };
query_kind_expr: 
        | KW_VERTEX { $$ = INIT_STRING_AST("vertex"); }
        | KW_EDGE { $$ = INIT_STRING_AST("edge"); }
        | KW_PATH { $$ = INIT_STRING_AST("path");}
        | a_graph_properties { $$ = $1; };
a_graph_expr: /* empty */
        | KW_IN COLON VAR_STRING
                {
                  $$ = GSinglecton::get<GStorageEngine>()->getGraph($3);
                  stm._errorCode = ECode_Graph_Not_Exist;
                  // if (!$$) {
                  //   printf("graph '%s' is not exist\n", $3);
                  // }
                };
a_path_plan:
        | OP_FROM COLON VAR_STRING COMMA OP_TO COLON VAR_STRING {}
        | OP_TO COLON VAR_STRING COMMA OP_FROM COLON VAR_STRING {};
where_expr: OP_WHERE COLON json { $$ = $3; };
string_list: VAR_STRING
              {
                size_t len = strlen($1) + 1;
                char* p = (char*)malloc(len);
                memcpy(p, $1, len);
                $$ = init_list(newast(NodeType::String, p, nullptr, nullptr));
              }
        | LEFT_SQUARE strings RIGHT_SQUARE
              {
                $$ = $2;
              }
        | property {};
// property_list: STAR { $$ = nullptr; }
//         | string_list { $$ = $1; };
strings:  VAR_STRING
              {
                size_t len = strlen($1) + 1;
                char* p = (char*)malloc(len);
                memcpy(p, $1, len);
                $$ = init_list(newast(NodeType::String, p, nullptr, nullptr));
              }
        | strings COMMA VAR_STRING
              {
                size_t len = strlen($3) + 1;
                char* p = (char*)malloc(len);
                memcpy(p, $3, len);
                gql_node* node = init_list(newast(NodeType::String, p, nullptr, nullptr));
                $$ = list_join($1, node);
              };
a_graph_properties:
        | graph_property {}
        | LEFT_SQUARE graph_properties RIGHT_SQUARE {};
graph_properties: 
        | graph_property {}
        | graph_properties COMMA graph_property {};
graph_property:
        | KW_VERTEX dot VAR_STRING {}
        | KW_EDGE dot VAR_STRING {}
        | KW_VERTEX dot function_call {}
        | KW_EDGE dot function_call {};
vertex_list: LEFT_SQUARE vertexes RIGHT_SQUARE
              {
                $$ = $2;
              };
vertexes: vertex { $$ = init_list($1); }
        | vertexes COMMA vertex
              {
                gql_node* node = init_list($3);
                $$ = list_join($1, node);
              };
vertex: LEFT_SQUARE VAR_STRING RIGHT_SQUARE
              {
                struct gast* id = INIT_STRING_AST("id");
                struct gast* value = INIT_STRING_AST($2);
                struct gast* left = newast(NodeType::Property, nullptr, id, value);
                $$ = newast(NodeType::Vertex, nullptr, left, nullptr);
              }
        | LEFT_SQUARE VAR_STRING COMMA json RIGHT_SQUARE
              {
                struct gast* id = INIT_STRING_AST("id");
                struct gast* value = INIT_STRING_AST($2);
                struct gast* left = newast(NodeType::Property, nullptr, id, value);
                $$ = newast(NodeType::Vertex, nullptr, left, $4);
              };
edge_list: LEFT_SQUARE edges RIGHT_SQUARE {$$ = $2;};
edges: edge { $$ = init_list($1); }
        | edge_list COMMA edge
              {
                gql_node* node = init_list($3);
                $$ = list_join($1, node);
              };
edge: LEFT_SQUARE VAR_STRING COMMA KW_RIGHT_RELATION COMMA VAR_STRING RIGHT_SQUARE
              {
                struct gast* id = INIT_STRING_AST("id");
                struct gast* from_value = INIT_STRING_AST($2);
                struct gast* from = newast(NodeType::Property, nullptr, id, from_value);
                struct gast* to_value = INIT_STRING_AST($6);
                struct gast* to = newast(NodeType::Property, nullptr, id, to_value);
                struct gast* link = INIT_STRING_AST("->");
                $$ = newast(NodeType::Edge, link, from, to);
              }
        | LEFT_SQUARE VAR_STRING COMMA KW_LEFT_RELATION COMMA VAR_STRING RIGHT_SQUARE
              {
                struct gast* id = INIT_STRING_AST("id");
                struct gast* from_value = INIT_STRING_AST($6);
                struct gast* from = newast(NodeType::Property, nullptr, id, from_value);
                struct gast* to_value = INIT_STRING_AST($2);
                struct gast* to = newast(NodeType::Property, nullptr, id, to_value);
                struct gast* link = INIT_STRING_AST("<-");
                $$ = newast(NodeType::Edge, link, from, to);
              }
        | LEFT_SQUARE VAR_STRING COMMA KW_BIDIRECT_RELATION COMMA VAR_STRING RIGHT_SQUARE
              {
                struct gast* id = INIT_STRING_AST("id");
                struct gast* from_value = INIT_STRING_AST($2);
                struct gast* from = newast(NodeType::Property, nullptr, id, from_value);
                struct gast* to_value = INIT_STRING_AST($6);
                struct gast* to = newast(NodeType::Property, nullptr, id, to_value);
                struct gast* link = INIT_STRING_AST("--");
                $$ = newast(NodeType::Edge, link, from, to);
              }
        | LEFT_SQUARE VAR_STRING RIGHT_SQUARE
              {
                struct gast* id = INIT_STRING_AST("id");
                struct gast* from_value = INIT_STRING_AST($2);
                struct gast* from = newast(NodeType::Property, nullptr, id, from_value);
                struct gast* to_value = INIT_STRING_AST($2);
                struct gast* to = newast(NodeType::Property, nullptr, id, to_value);
                struct gast* link = INIT_STRING_AST("--");
                $$ = newast(NodeType::Edge, link, from, to);
              };
json: value { $$ = $1; };
value: object { $$ = $1; }
        | VAR_DECIMAL
              {
                double* val = (double*)malloc(sizeof(double));
                *val = $1;
                $$ = newast(NodeType::Number, val, nullptr, nullptr);
              }
        | VAR_INTEGER
              {
                double* val = (double*)malloc(sizeof(double));
                *val = $1;
                $$ = newast(NodeType::Number, val, nullptr, nullptr);
              }
        | array { $$ = $1; }
        | VAR_BASE64
              {
                std::vector<uint8_t> bin = gql::base64_decode($1);
                free($1);
                uint8_t* p = (uint8_t*)malloc(sizeof(uint8_t)*bin.size() + sizeof(size_t));
                size_t* size = (size_t*)p;
                *size = bin.size();
                memcpy(p + sizeof(size_t), bin.data(), bin.size());
                $$ = newast(NodeType::Binary, p, nullptr, nullptr);
              }
        | VAR_DATETIME {}
        | VAR_STRING { $$ = INIT_STRING_AST($1); };
object: RANGE_BEGIN properties RANGE_END
            {
              $$ = newast(NodeType::ObjectExpression, $2, nullptr, nullptr);
            };
properties: property {
              gql_node* list = init_list($1);
              $$ = newast(NodeType::ArrayExpression, list, nullptr, nullptr);
            }
        | properties COMMA property
              {
                gql_node* list = init_list($3);
                gql_node* head = list_join((gql_node*)($1->_value), list);
                $1->_value = head;
                $$ = $1;
              };
property: VAR_STRING COLON value
              {
                INIT_ITEM($1, $3, NodeType::Property);
                $$ = root;
              }
        | KW_ID COLON VAR_STRING
              {
                struct gast* key = INIT_STRING_AST("id");
                struct gast* value = INIT_STRING_AST($3);
                $$ = newast(NodeType::Property, nullptr, key, value);
              }
        | OP_GREAT_THAN_EQUAL COLON VAR_INTEGER
              {
                struct gast* key = INIT_STRING_AST("gte");
                struct gast* value = INIT_LITERAL_AST($3, NodeType::Integer);
                $$ = newast(NodeType::Property, nullptr, key, value);
              }
        | OP_LESS_THAN_EQUAL COLON VAR_INTEGER
              {
                struct gast* key = INIT_STRING_AST("lte");
                struct gast* value = INIT_LITERAL_AST($3, NodeType::Integer);
                $$ = newast(NodeType::Property, nullptr, key, value);
              }
        | OP_GREAT_THAN COLON VAR_INTEGER
              {
                struct gast* key = INIT_STRING_AST("gt");
                struct gast* value = INIT_LITERAL_AST($3, NodeType::Integer);
                $$ = newast(NodeType::Property, nullptr, key, value);
              }
        | OP_LESS_THAN COLON VAR_INTEGER
              {
                struct gast* key = INIT_STRING_AST("lt");
                struct gast* value = INIT_LITERAL_AST($3, NodeType::Integer);
                $$ = newast(NodeType::Property, nullptr, key, value);
              }
        | OP_AND COLON array
              {
                struct gast* key = INIT_STRING_AST("and");
                struct gast* value = INIT_LITERAL_AST($3, NodeType::ArrayExpression);
                $$ = newast(NodeType::Property, nullptr, key, value);
              }
        | OP_OR COLON array
              {
                struct gast* key = INIT_STRING_AST("or");
                struct gast* value = INIT_LITERAL_AST($3, NodeType::ArrayExpression);
                $$ = newast(NodeType::Property, nullptr, key, value);
              }
        | a_link_condition {}
        // | KW_RIGHT_RELATION COLON VAR_INTEGER
        //       {
        //         struct gast* key = INIT_STRING_AST("rac"); // right array count
        //         struct gast* value = INIT_LITERAL_AST($3, NodeType::Integer);
        //         $$ = newast(NodeType::Property, nullptr, key, value);
        //       }
        // | KW_LEFT_RELATION COLON VAR_INTEGER
        //       {
        //         struct gast* key = INIT_STRING_AST("lac");  // left arraw count
        //         struct gast* value = INIT_LITERAL_AST($3, NodeType::Integer);
        //         $$ = newast(NodeType::Property, nullptr, key, value);
        //       }
        // | KW_BIDIRECT_RELATION COLON VAR_INTEGER
        //       {
        //         struct gast* key = INIT_STRING_AST("ac"); // arraw count
        //         struct gast* value = INIT_LITERAL_AST($3, NodeType::Integer);
        //         $$ = newast(NodeType::Property, nullptr, key, value);
        //       }
        // | OP_TO COLON VAR_STRING
        //       {
        //         struct gast* key = INIT_STRING_AST("->");
        //         struct gast* value = INIT_LITERAL_AST($3, NodeType::String);
        //         $$ = newast(NodeType::Property, nullptr, key, value);
        //       }
        | ;
array:    LEFT_SQUARE RIGHT_SQUARE { $$ = nullptr; }
        | LEFT_SQUARE values RIGHT_SQUARE
              {
                $$ = $2;
              }
        | LEFT_SQUARE RIGHT_SQUARE { $$ = nullptr; };
values: value {
                gql_node* list = init_list($1);
                $$ = newast(NodeType::ArrayExpression, list, nullptr, nullptr);
              }
        | values COMMA value
              {
                gql_node* list = init_list($3);
                gql_node* head = list_join((gql_node*)($1->_value), list);
                $1->_value = head;
                $$ = $1;
              }
        | values COMMA KW_REST {};
a_link_condition: 
        | a_edge COLON a_value
              {

              };
a_edge:
        | KW_RIGHT_RELATION { $$ = INIT_STRING_AST("->");}
        | KW_LEFT_RELATION { $$ = INIT_STRING_AST("<-"); }
        | KW_BIDIRECT_RELATION { $$ = INIT_STRING_AST("--"); };
a_value:
        | VAR_STRING { $$ = INIT_STRING_AST($1); }
        | VAR_INTEGER { $$ = INIT_BASIC_TYPE_AST($1, NodeType::Integer); };
function_call:
        | VAR_STRING function_params {};
function_params:
        | PARAM_BEGIN PARAM_END {}
        | PARAM_BEGIN STAR PARAM_END {}
        | PARAM_BEGIN string_list PARAM_END {}
test: json {printf("Unknow Input\n");};
%%
