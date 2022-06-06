%define api.pure full
%locations
%define parse.error verbose
// %define api.prefix {gql_yy}
%param { yyscan_t scanner }
%code requires {
  typedef void* yyscan_t;
}

%code top {
#include <stdio.h>
#include <set>
#include <fmt/format.h>
#include "Error.h"
#include "gql/creation.h"
#include "gql/query.h"
#include "gql/upset.h"
#include "Type/Binary.h"
#include "base/lang/lang.h"
#include "base/VertexVisitor.h"
#include "base/EdgeVisitor.h"

#define MAX_VARIANT_SIZE  32
#define GET_GRAPH(name)  \
  GGraph* pGraph = stm._graph->getGraph(name);\
  if (!pGraph) {\
    break;\
  }
} // top

%lex-param {GVirtualEngine& stm}
%parse-param {GVirtualEngine& stm}
%union {
  struct GASTNode* __node;
  char var_name[MAX_VARIANT_SIZE];
  float __f;
  char* __c;
  size_t __offset;
  time_t __datetime;
  int32_t __int;
  nlohmann::json* __json;
}

%code {
#define YY_DECL \
       int yylex(YYSTYPE* yylval_param, YYLTYPE* yylloc_param, void* yyscanner, GVirtualEngine& stm)

void yyerror(YYLTYPE* yyllocp, yyscan_t unused, GVirtualEngine& stm, const char* msg) {
  std::string err_index;
  if (stm._errIndx) {
    std::string err_offset(stm._errIndx - 1, ' ');
    err_index += err_offset + "~";
  }
  printf("\033[22;31mError:\t%s:\033[22;0m\n\t%s\n\t%s\n",
    msg, stm.gql().c_str(), err_index.c_str());
}

void release_vertex_callback(gql::vertex*& v) {}

struct GASTNode* INIT_STRING_AST(const char* key) {
  size_t len = strlen(key) + 1;
  // void* s = malloc(len);
  // memcpy(s, key, len);
  // printf("|-> %s\n", key);
  GLiteral* str = new GLiteralString(key, len);
  return NewAst(NodeType::Literal, str, nullptr, 0);
}

template<typename T>
struct GASTNode* INIT_NUMBER_AST(T& v) {
  GLiteral* number = new GLiteralNumber(v);
  return NewAst(NodeType::Literal, number, nullptr, 0);
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
%token KW_AST KW_ID KW_GRAPH KW_COMMIT
%token KW_CREATE KW_DROP KW_IN KW_REMOVE KW_UPSET KW_LEFT_RELATION KW_RIGHT_RELATION KW_BIDIRECT_RELATION KW_REST KW_DELETE
%token OP_QUERY KW_INDEX KW_GROUP OP_WHERE
%token CMD_SHOW 
%token OP_GREAT_THAN OP_LESS_THAN OP_GREAT_THAN_EQUAL OP_LESS_THAN_EQUAL OP_EQUAL OP_AND OP_OR
%token FN_COUNT
%token dot
%token limit profile
%token <__node> KW_VERTEX
%token <__node> KW_EDGE
%token <__node> KW_PATH

%type <__node> a_graph_expr
%type <__node> json
%type <__node> value number
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
%type <__node> creation
%type <__node> upset_vertexes
%type <__node> upset_edges
%type <__node> a_simple_query
%type <__node> query_kind_expr
%type <__node> query_kind
%type <__node> a_graph_properties
%type <__node> a_edge
%type <__node> a_link_condition
%type <__node> a_value
%type <__node> drop_graph
%type <__node> remove_vertexes
%type <__node> vertex_list vertexes string_list strings property_list edges edge_list

%start line_list

%%
line_list: line
          | line_list line
          ;
    line: gql {
          stm._errorCode = stm.execAST($1);
        }
        | utility_cmd { stm._cmdtype = GQL_Util; }
        | {}
        ;
    gql: creation
          {
            GGQLExpression* expr = new GGQLExpression();
            $$ = NewAst(NodeType::GQLExpression, expr, $1, 1);
            stm._cmdtype = GQL_Creation;
          }
        | a_simple_query { $$ = $1; stm._cmdtype = GQL_Query; }
        | upset_vertexes
          {
            GGQLExpression* expr = new GGQLExpression();
            $$ = NewAst(NodeType::GQLExpression, expr, $1, 1);
            stm._cmdtype = GQL_Upset;
          }
        | upset_edges
          {
            GGQLExpression* expr = new GGQLExpression();
            $$ = NewAst(NodeType::GQLExpression, expr, $1, 1);
            stm._cmdtype = GQL_Upset;
          }
        | remove_vertexes { $$ = $1; stm._cmdtype = GQL_Remove; }
        | drop_graph
          {
            GGQLExpression* expr = new GGQLExpression();
            $$ = NewAst(NodeType::GQLExpression, expr, $1, 1);
            stm._cmdtype = GQL_Drop;
          }
        | dump { stm._cmdtype = GQL_Util; }
        ;
utility_cmd: CMD_SHOW KW_GRAPH
          {
            // std::vector<std::string> vg = stm._graph->getGraphs();
            // gqlite_result results;
            // init_result_info(results, vg);
            // stm._result_callback(&results);
            // release_result_info(results);
            stm._errorCode = ECode_Success;
          }
        | CMD_SHOW KW_GRAPH VAR_STRING
          {
            // GGraphInterface* g = stm._graph->getGraph($3);
            // const GraphProperty& props = g->property();
            // char buff[256] = {0};
            // sprintf(buff, "show graph [%s]:\n", $3);
            // std::string str(buff);
            // for (auto& name: props._indexes) {
            //   memset(buff, 0, 256);
            //   sprintf(buff, "\tindex: %s\n", name.c_str());
            //   str += buff;
            // }
            // gqlite_result results;
            // init_result_info(results, {str});
            // stm._result_callback(&results);
            // release_result_info(results);
            stm._errorCode = ECode_Success;
          }
        | KW_AST gql
          {
            fmt::print("AST:\n");
            DumpAst($2);
            stm._cmdtype = GQL_Util;
          }
        | profile gql {};
creation: RANGE_BEGIN KW_CREATE COLON VAR_STRING RANGE_END
            {
              // stm._graph->openGraph($4);
              GCreateStmt* createStmt = new GCreateStmt($4, nullptr);
              $$ = NewAst(NodeType::CreationStatement, createStmt, nullptr, 0);
              stm._errorCode = ECode_Success;
            }
        | RANGE_BEGIN KW_CREATE COLON VAR_STRING COMMA KW_INDEX COLON string_list RANGE_END
            {
              // stm._graph->openGraph($4);
              // GGraph* g = stm._graph->getGraph($4);
              // gql_node* cur = $8;
              // do {
              //   GASTNode* s = (struct GASTNode*)(cur->_value);
              //   std::string value = GET_STRING_VALUE(s);
              //   creation::createInvertIndex(stm._graph, g, value.c_str());
              //   cur = cur->_next;
              // } while(cur);

              GCreateStmt* createStmt = new GCreateStmt($4, $8);
              $$ = NewAst(NodeType::CreationStatement, createStmt, nullptr, 0);
              stm._errorCode = ECode_Success;
            }
        | RANGE_BEGIN KW_CREATE COLON VAR_STRING COMMA KW_INDEX COLON function_call RANGE_END {};
dump: RANGE_BEGIN KW_AST COLON VAR_STRING RANGE_END
            {
              // if (strlen($4) == 0) {
              //   break;
              // }
              // std::vector<std::string> gs = stm._graph->getGraphs();
              // std::string dump;
              // for (std::string& gname: gs) {
              //   dump += creation::generateGraphScript(gname);
              //   GGraph* g = stm._graph->getGraph(gname.c_str());
              //   if (!g) {
              //     continue;
              //   }
              //   std::string vertexes = upset::exportVertexes(g);
              //   if (vertexes.size()) {
              //     dump += vertexes;
              //     dump += ",";
              //   }
              //   std::string edges = upset::exportEdges(g);
              //   if (edges.size()) {
              //     dump += edges;
              //     dump += ",";
              //   }
              // }
              // dump.pop_back();
              // FILE* f = fopen($4, "w");
              // if (!f) break;
              // fwrite(dump.c_str(), 1, dump.size(), f);
              // fclose(f);
              // gprint(LITE_DEBUG, "dump graph %s finish", $4);
              stm._errorCode = ECode_Success;
            };
upset_vertexes: RANGE_BEGIN KW_UPSET COLON VAR_STRING COMMA KW_VERTEX COLON vertex_list RANGE_END
              {
                // struct GASTNode* g = INIT_STRING_AST($4);
                // $$ = NewAst(NodeType::UpsetStatement, $8, g, nullptr);
                // GET_GRAPH($4);
                // gql_node* cur = $8;
                // ASTVertexUpdateVisitor visitor;
                // while(cur) {
                //   GASTNode* pv = (GASTNode*)(cur->_value);
                //   traverse(pv, &visitor);
                //   cur = cur->_next;
                // }
                // release_list($8, release_vertex_callback);
                // stm._graph->finishUpdate(pGraph);

                GUpsetStmt* upsetStmt = new GUpsetStmt($4);
                $$ = NewAst(NodeType::UpsetStatement, upsetStmt, $8, 1);
              };
remove_vertexes: RANGE_BEGIN KW_REMOVE COLON VAR_STRING COMMA KW_VERTEX COLON array RANGE_END
              {
                struct GASTNode* g = INIT_STRING_AST($4);
                // $$ = NewAst(NodeType::RemoveStatement, $8, g, nullptr);
                // GET_GRAPH($4);
                // gql_node* list = as_array($8);
                // gql_node* current = list;
                // while(current) {
                //   GASTNode* node = (struct GASTNode*)current->_value;
                //   // if (node->)
                //   char* nodeid = (char*)node->_value;
                //   stm._graph->dropNode(pGraph, nodeid);
                //   current = current->_next;
                // }
                // stm._graph->finishUpdate(pGraph);
              };
upset_edges: RANGE_BEGIN KW_UPSET COLON VAR_STRING COMMA KW_EDGE COLON edge_list RANGE_END
              {
                // struct GASTNode* g = INIT_STRING_AST($4);
                // $$ = NewAst(NodeType::UpsetStatement, $8, g, nullptr);
                // GET_GRAPH($4);
                // gql_node* cur = $8;
                // ASTEdgeUpdateVisitor visitor;
                // while(cur) {
                //   GASTNode* pv = (GASTNode*)(cur->_value);
                //   traverse(pv, &visitor);
                //   cur = cur->_next;
                // }
                GUpsetStmt* upsetStmt = new GUpsetStmt($4);
                $$ = NewAst(NodeType::UpsetStatement, upsetStmt, $8, 1);
              };
drop_graph: RANGE_BEGIN KW_DROP COLON VAR_STRING RANGE_END
              {
                struct GASTNode* g = INIT_STRING_AST($4);
                // $$ = NewAst(NodeType::UpsetStatement, g, nullptr, nullptr);
                // GET_GRAPH($4);
                // stm._graph->dropGraph(pGraph);
              };
a_simple_query: 
          RANGE_BEGIN query_kind COMMA a_graph_expr RANGE_END
                {
                  // if (!$4) break;
                  // std::vector<VertexID> ids = GSinglecton::get<GStorageEngine>()->getNodes($4);
                  // gqlite_result results;
                  // query::get_vertexes($4, ids, results);
                  // query::filter_property(results, $2);
                  // results.type = gqlite_result_type_node;
                  // stm._result_callback(&results);
                  // query::release_vertexes(results);
                  GQueryStmt* queryStmt = new GQueryStmt($2, $4, nullptr);
                  $$ = NewAst(NodeType::QueryStatement, queryStmt, nullptr, 0);
                  stm._errorCode = ECode_Success;
                }
        | RANGE_BEGIN query_kind COMMA a_graph_expr COMMA where_expr RANGE_END
                {
                  // const GraphProperty& properties = $4->property();
                  // ASTVertexQueryVisitor visitor;
                  // traverse($6, &visitor);
                  // std::set<VertexID> sIds;
                  // stm._errorCode = $4->queryVertex(sIds, visitor.conditions());
                  // if(!stm._errorCode && sIds.size() == 0) break;
                  // gqlite_result results;
                  // results.type = gqlite_result_type_node;
                  // query::get_vertexes($4, sIds, results);
                  // query::filter_property(results, $2);
                  // stm._result_callback(&results);
                  // query::release_vertexes(results);
                  GQueryStmt* queryStmt = new GQueryStmt($2, $4, $6);
                  $$ = NewAst(NodeType::QueryStatement, queryStmt, nullptr, 0);
                  stm._errorCode = ECode_Success;
                };
query_kind: OP_QUERY COLON query_kind_expr { $$ = $3; }
        |   OP_QUERY COLON match_expr {};
query_kind_expr: 
          KW_VERTEX { $$ = INIT_STRING_AST("vertex"); }
        | KW_EDGE { $$ = INIT_STRING_AST("edge"); }
        | KW_PATH { $$ = INIT_STRING_AST("path");}
        | a_graph_properties { $$ = $1; };
match_expr: //{->: 'alias'}
          RANGE_BEGIN a_match RANGE_END {};
a_match:  a_relation_match {}
        | a_vertex_match COMMA a_relation_match COMMA a_vertex_match {};
a_relation_match: KW_RIGHT_RELATION COLON VAR_STRING {}
        | KW_LEFT_RELATION COLON VAR_STRING {};
a_vertex_match: KW_VERTEX COLON VAR_STRING {};
a_graph_expr:
          KW_IN COLON VAR_STRING
                {
                  // stm._errorCode = ECode_Graph_Not_Exist;
                  $$ = INIT_STRING_AST($3);
                };
where_expr: OP_WHERE COLON json { $$ = $3; };
string_list: VAR_STRING
              {
                struct GASTNode* node = INIT_STRING_AST($1);
                $$ = NewAst(NodeType::ArrayExpression, node, nullptr, 0);
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
                struct GASTNode* node = INIT_STRING_AST($1);
                $$ = NewAst(NodeType::ArrayExpression, node, nullptr, 0);
              }
        | strings COMMA VAR_STRING
              {
                GASTNode* node = NewAst(NodeType::ArrayExpression, INIT_STRING_AST($3), nullptr, 0);
                $$ = ListJoin($1, node);
              };
number: VAR_DECIMAL { $$ = INIT_NUMBER_AST($1); }
        | VAR_INTEGER { $$ = INIT_NUMBER_AST($1); };
a_graph_properties:
          graph_property {}
        | LEFT_SQUARE graph_properties RIGHT_SQUARE {};
graph_properties: 
          graph_property {}
        | graph_properties COMMA graph_property {};
graph_property:
          KW_VERTEX dot VAR_STRING {}
        | KW_EDGE dot VAR_STRING {}
        | KW_VERTEX dot function_call {}
        | KW_EDGE dot function_call {};
vertex_list: LEFT_SQUARE vertexes RIGHT_SQUARE
              {
                $$ = $2;
              };
vertexes: vertex
              {
                $$ = $1;
              }
        | vertexes COMMA vertex
              {
                $$ = ListJoin($1, $3);
              };
vertex: LEFT_SQUARE VAR_STRING RIGHT_SQUARE
              {
                struct GASTNode* value = INIT_STRING_AST($2);
                $$ = NewAst(NodeType::ArrayExpression, value, nullptr, 0);
              }
        | LEFT_SQUARE VAR_STRING COMMA json RIGHT_SQUARE
              {
                struct GASTNode* value = INIT_STRING_AST($2);
                GASTNode* node = NewAst(NodeType::ArrayExpression, value, nullptr, 0);
                GASTNode* jsn = NewAst(NodeType::ArrayExpression, $4, nullptr, 0);
                GASTNode* vertex = ListJoin(node, jsn);
                GVertexDeclaration* decl = new GVertexDeclaration(vertex);
                $$ = NewAst(NodeType::VertexDeclaration, decl, nullptr, 0);
              };
edge_list: LEFT_SQUARE edges RIGHT_SQUARE {$$ = $2;};
edges: edge { $$ = $1; }
        | edges COMMA edge
              {
                $$ = ListJoin($1, $3);
              };
edge: LEFT_SQUARE VAR_STRING COMMA a_edge COMMA VAR_STRING RIGHT_SQUARE
              {
                struct GASTNode* from_value = INIT_STRING_AST($2);
                struct GASTNode* to_value = INIT_STRING_AST($6);
                GEdgeDeclaration* edge = new GEdgeDeclaration(from_value, to_value, $4);
                $$ = NewAst(NodeType::EdgeDeclaration, edge, nullptr, 0);
              }
        | LEFT_SQUARE VAR_STRING RIGHT_SQUARE
              {
                struct GASTNode* from_value = INIT_STRING_AST($2);
                GEdgeDeclaration* edge = new GEdgeDeclaration(from_value, from_value, INIT_STRING_AST("--"));
                $$ = NewAst(NodeType::EdgeDeclaration, edge, nullptr, 0);
              };
json: value { $$ = $1; };
value: object { $$ = $1; }
        | array { $$ = $1; }
        | number { $$ = $1; }
        | VAR_BASE64
              {
                GLiteralBinary* bin = new GLiteralBinary($1, "b64");
                $$ = NewAst(NodeType::Literal, bin, nullptr, 0);
              }
        | VAR_DATETIME {}
        | VAR_STRING
              {
                $$ = INIT_STRING_AST($1); };
object: RANGE_BEGIN properties RANGE_END
            {
              $$ = NewAst(NodeType::ObjectExpression, $2, nullptr, 0);
            };
properties: property {
              $$ = NewAst(NodeType::ArrayExpression, $1, nullptr, 0);
            }
        | properties COMMA property
              {
                GASTNode* node = NewAst(NodeType::ArrayExpression, $3, nullptr, 0);
                $$ = ListJoin($1, node);
              };
property: VAR_STRING COLON value
              {
                GProperty* prop = new GProperty($1, $3);
                $$ = NewAst(NodeType::Property, prop, nullptr, 0);
              }
        | KW_ID COLON VAR_STRING
              {
                struct GASTNode* value = INIT_STRING_AST($3);
                GProperty* prop = new GProperty("id", value);
                $$ = NewAst(NodeType::Property, prop, nullptr, 0);
              }
        | OP_GREAT_THAN_EQUAL COLON number
              {
                GProperty* prop = new GProperty("gte", $3);
                $$ = NewAst(NodeType::BinaryExpression, prop, nullptr, 0);
              }
        | OP_LESS_THAN_EQUAL COLON number
              {
                GProperty* prop = new GProperty("lte", $3);
                $$ = NewAst(NodeType::BinaryExpression, prop, nullptr, 0);
              }
        | OP_GREAT_THAN COLON number
              {
                GProperty* prop = new GProperty("gt", $3);
                $$ = NewAst(NodeType::BinaryExpression, prop, nullptr, 0);
              }
        | OP_LESS_THAN COLON number
              {
                GProperty* prop = new GProperty("lt", $3);
                $$ = NewAst(NodeType::BinaryExpression, prop, nullptr, 0);
              }
        | OP_AND COLON array
              {
                struct GASTNode* key = INIT_STRING_AST("and");
                // struct GASTNode* value = INIT_NUMBER_AST($3, NodeType::ArrayExpression);
                // $$ = NewAst(NodeType::Property, nullptr, key, value);
              }
        | OP_OR COLON array
              {
                struct GASTNode* key = INIT_STRING_AST("or");
                // struct GASTNode* value = INIT_NUMBER_AST($3, NodeType::ArrayExpression);
                // $$ = NewAst(NodeType::Property, nullptr, key, value);
              }
        | a_link_condition {}
        // | KW_RIGHT_RELATION COLON VAR_INTEGER
        //       {
        //         struct GASTNode* key = INIT_STRING_AST("rac"); // right array count
        //         struct GASTNode* value = INIT_LITERAL_AST($3, NodeType::Integer);
        //         $$ = NewAst(NodeType::Property, nullptr, key, value);
        //       }
        // | KW_LEFT_RELATION COLON VAR_INTEGER
        //       {
        //         struct GASTNode* key = INIT_STRING_AST("lac");  // left arraw count
        //         struct GASTNode* value = INIT_LITERAL_AST($3, NodeType::Integer);
        //         $$ = NewAst(NodeType::Property, nullptr, key, value);
        //       }
        // | KW_BIDIRECT_RELATION COLON VAR_INTEGER
        //       {
        //         struct GASTNode* key = INIT_STRING_AST("ac"); // arraw count
        //         struct GASTNode* value = INIT_LITERAL_AST($3, NodeType::Integer);
        //         $$ = NewAst(NodeType::Property, nullptr, key, value);
        //       }
        // | OP_TO COLON VAR_STRING
        //       {
        //         struct GASTNode* key = INIT_STRING_AST("->");
        //         struct GASTNode* value = INIT_LITERAL_AST($3, NodeType::String);
        //         $$ = NewAst(NodeType::Property, nullptr, key, value);
        //       }
        | ;
array:    LEFT_SQUARE RIGHT_SQUARE { $$ = nullptr; }
        | LEFT_SQUARE values RIGHT_SQUARE
              {
                $$ = $2;
              };
values: value {
                $$ = NewAst(NodeType::ArrayExpression, $1, nullptr, 0);
              }
        | values COMMA value
              {
                GASTNode* node = NewAst(NodeType::ArrayExpression, $3, nullptr, 0);
                $$ = ListJoin($1, node);
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
        | VAR_DECIMAL { $$ = INIT_NUMBER_AST($1); }
        | VAR_INTEGER { $$ = INIT_NUMBER_AST($1); };
function_call:
        | VAR_STRING function_params {};
function_params:
        | PARAM_BEGIN PARAM_END {}
        | PARAM_BEGIN STAR PARAM_END {}
        | PARAM_BEGIN string_list PARAM_END {}
        ;
%%
