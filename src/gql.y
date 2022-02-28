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
struct gast* INIT_LITERAL_AST(T& v, NodeType type) {
  void* value = malloc(sizeof(T));
  memcpy(value, &v, sizeof(T));
  return newast(type, value, nullptr, nullptr);
}

nlohmann::json* get_or_create_json_array(nlohmann::json* item) {
  if (!item->is_array()) {
    nlohmann::json* j = new nlohmann::json();
    j->push_back(*item);
    delete item;
    return j;
  }
  return item;
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
%token KW_CREATE KW_VERTEX KW_EDGE KW_DROP KW_IN KW_REMOVE KW_UPSET KW_LEFT_RELATION KW_RIGHT_RELATION KW_BIDIRECT_RELATION KW_PATH KW_REST KW_DELETE
%token OP_QUERY OP_WALK OP_FROM KW_INDEX KW_GROUP OP_WHERE OP_TO
%token CMD_SHOW 
%token OP_GREAT_THAN OP_LESS_THAN OP_GREAT_THAN_EQUAL OP_LESS_THAN_EQUAL OP_EQUAL OP_AND OP_OR
%token FN_COUNT

%type <__list> vertex_list vertexes string_list strings property_list edges edge_list
%type <_g> from_graph_expr
%type <__node> json value values object array properties where_expr function_call property vertex edge gql upset_vertexes upset_edges

%start line_list

%%
line_list: line
          | line_list line
          ;
    line: gql {}
        | utility_cmd {};
    gql: creation  { printf("create graph success\n"); }
        | query {}
        | walk {}
        | upset_vertexes { $$ = $1; }
        | upset_edges { $$ = $1; }
        | remove_vertexes {}
        | drop_graph {}
        | dump {}
        | test {}
        ;

utility_cmd: CMD_SHOW KW_GRAPH
          {
            std::vector<std::string> vg = GSinglecton::get<GStorageEngine>()->getGraphs();
            gqlite_result results;
            results.count = vg.size();
            results.graphs = (char**)malloc(results.count * sizeof(char*));
            for (size_t idx = 0; idx < results.count; ++idx) {
              results.graphs[idx] = (char*)malloc(vg[idx].size() + 1);
              memcpy(results.graphs[idx], vg[idx].data(), vg[idx].size() + 1);
            }
            stm._result_callback(&results);
            for (size_t idx = 0; idx < results.count; ++idx) {
              free(results.graphs[idx]);
            }
            free(results.graphs);
          }
        | CMD_SHOW KW_GRAPH VAR_STRING
          {
            GGraph* g = GSinglecton::get<GStorageEngine>()->getGraph($3);
            const GraphProperty& props = g->property();
            printf("show graph [%s]:\n", $3);
            for (auto& name: props._indexes) {
              printf("\tindex: %s\n", name.c_str());
            }
          }
          | KW_DUMP gql {};
creation: RANGE_BEGIN KW_CREATE VAR_STRING RANGE_END
            {
              GSinglecton::get<GStorageEngine>()->openGraph($3);
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
              gprint(LITE_DEBUG, "dump graph %s finish", $4);
            };
query: RANGE_BEGIN OP_QUERY COLON property_list COMMA from_graph_expr RANGE_END
              {
                if (!$6) break;
                std::vector<VertexID> ids = GSinglecton::get<GStorageEngine>()->getNodes($6);
                gqlite_result results;
                query::get_vertexes($6, ids, results);
                query::filter_property(results, $4);
                stm._result_callback(&results);
                query::release_vertexes(results);
              }
        | RANGE_BEGIN OP_QUERY COLON property_list COMMA KW_PATH COLON array COMMA from_graph_expr RANGE_END {}
        | RANGE_BEGIN OP_QUERY COLON property_list COMMA from_graph_expr COMMA where_expr RANGE_END
              {
                if (!$6) break;
                // check if index table exist or not
                const GraphProperty& properties = $6->property();
                ASTVertexQueryVisitor visitor;
                traverse($8, &visitor);
                std::set<VertexID> sIds;
                if(!$6->queryVertex(sIds, visitor.conditions()) && sIds.size() == 0) break;
                gqlite_result results;
                query::get_vertexes($6, sIds, results);
                query::filter_property(results, $4);
                stm._result_callback(&results);
                query::release_vertexes(results);
              };
walk: RANGE_BEGIN OP_WALK COLON VAR_STRING COMMA KW_IN COLON VAR_STRING COMMA OP_FROM COLON VAR_STRING COMMA OP_TO COLON VAR_STRING RANGE_END
              {};
where_expr: OP_WHERE COLON json { $$ = $3; };
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
              };
drop_graph: RANGE_BEGIN KW_DROP COLON VAR_STRING RANGE_END
              {
                GET_GRAPH($4);
                GSinglecton::get<GStorageEngine>()->dropGraph(pGraph);
              };
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
              };
property_list: STAR { $$ = nullptr; }
        | string_list { $$ = $1; };
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
from_graph_expr: OP_FROM COLON VAR_STRING
              {
                $$ = GSinglecton::get<GStorageEngine>()->getGraph($3);
                if (!$$) {
                  printf("graph '%s' is not exist\n", $3);
                }
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
        | KW_RIGHT_RELATION COLON VAR_INTEGER
              {
                struct gast* key = INIT_STRING_AST("->");
                struct gast* value = INIT_LITERAL_AST($3, NodeType::WalkStatement);
                $$ = newast(NodeType::Property, nullptr, key, value);
              }
        | KW_LEFT_RELATION COLON VAR_INTEGER
              {
                struct gast* key = INIT_STRING_AST("<-");
                struct gast* value = INIT_LITERAL_AST($3, NodeType::WalkStatement);
                $$ = newast(NodeType::Property, nullptr, key, value);
              }
        | KW_BIDIRECT_RELATION COLON VAR_INTEGER
              {
                struct gast* key = INIT_STRING_AST("--");
                struct gast* value = INIT_LITERAL_AST($3, NodeType::WalkStatement);
                $$ = newast(NodeType::Property, nullptr, key, value);
              };
array: LEFT_SQUARE values RIGHT_SQUARE
              {
                $$ = $2;
              };
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
function_call: VAR_STRING PARAM_BEGIN PARAM_END {}
        | VAR_STRING PARAM_BEGIN string_list PARAM_END {};
test: json {printf("Unknow Input\n");};
%%
