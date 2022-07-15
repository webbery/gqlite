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
#include "base/lang/AST.h"

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

}

%token RANGE_BEGIN RANGE_END COLON QUOTE COMMA LEFT_SQUARE RIGHT_SQUARE STAR CR PARAM_BEGIN PARAM_END
%token <var_name> VAR_STRING VAR_HASH
%token <__f> VAR_DECIMAL
%token <__c> VAR_BASE64
%token <__int> VAR_INTEGER
%token <__int> VAR_DATETIME
%token KW_AST KW_ID KW_GRAPH KW_COMMIT
%token KW_CREATE KW_DROP KW_IN KW_REMOVE KW_UPSET left_arrow right_arrow KW_BIDIRECT_RELATION KW_REST KW_DELETE
%token OP_QUERY KW_INDEX OP_WHERE
%token group
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
%type <__node> function_call function_params
%type <__node> property
%type <__node> vertex
%type <__node> edge
%type <__node> gql
%type <__node> creation
%type <__node> upset_vertexes
%type <__node> upset_edges
%type <__node> a_simple_query
%type <__node> query_kind_expr a_match   a_relation_match  match_expr
%type <__node> query_kind
%type <__node> a_graph_properties graph_property graph_properties
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
          FreeAst($1);
        }
        | utility_cmd { stm._cmdtype = GQL_Util; }
        | error '\n' {
          printf("++++++Error\n");
        }
        ;
    gql: creation
          {
            GGQLExpression* expr = new GGQLExpression();
            $$ = NewAst(NodeType::GQLExpression, expr, $1, 1);
            stm._cmdtype = GQL_Creation;
          }
        | a_simple_query {
            GGQLExpression* expr = new GGQLExpression();
            $$ = NewAst(NodeType::GQLExpression, expr, $1, 1);
            stm._cmdtype = GQL_Query;
          }
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
            // DumpAst($2);
            GViewVisitor visitor;
            std::list<NodeType> ln;
            accept($2, visitor, ln);
            FreeAst($2);
            stm._cmdtype = GQL_Util;
          }
        | profile gql {};
creation: RANGE_BEGIN KW_CREATE COLON VAR_STRING RANGE_END
            {
              GCreateStmt* createStmt = new GCreateStmt($4, nullptr);
              $$ = NewAst(NodeType::CreationStatement, createStmt, nullptr, 0);
              stm._errorCode = ECode_Success;
            }
        | RANGE_BEGIN KW_CREATE COLON VAR_STRING COMMA KW_INDEX COLON string_list RANGE_END
            {
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
                GUpsetStmt* upsetStmt = new GUpsetStmt($4, $8);
                $$ = NewAst(NodeType::UpsetStatement, upsetStmt, nullptr, 0);
              };
remove_vertexes: RANGE_BEGIN KW_REMOVE COLON VAR_STRING COMMA KW_VERTEX COLON array RANGE_END
              {
                // struct GASTNode* g = INIT_STRING_AST($4);
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
                GRemoveStmt* rmStmt = new GRemoveStmt($4);
                $$ = NewAst(NodeType::RemoveStatement, rmStmt, $8, 1);
              };
upset_edges: RANGE_BEGIN KW_UPSET COLON VAR_STRING COMMA KW_EDGE COLON edge_list RANGE_END
              {
                GUpsetStmt* upsetStmt = new GUpsetStmt($4, $8);
                $$ = NewAst(NodeType::UpsetStatement, upsetStmt, nullptr, 0);
              };
drop_graph: RANGE_BEGIN KW_DROP COLON VAR_STRING RANGE_END
              {
                // struct GASTNode* g = INIT_STRING_AST($4);
                // $$ = NewAst(NodeType::UpsetStatement, g, nullptr, nullptr);
                // GET_GRAPH($4);
                // stm._graph->dropGraph(pGraph);
                GDropStmt* dropStmt = new GDropStmt($4);
                $$ = NewAst(NodeType::DropStatement, dropStmt, nullptr, 0);
              };
a_simple_query: 
          RANGE_BEGIN query_kind COMMA a_graph_expr RANGE_END
                {
                  GQueryStmt* queryStmt = new GQueryStmt($2, $4, nullptr);
                  $$ = NewAst(NodeType::QueryStatement, queryStmt, nullptr, 0);
                  stm._errorCode = ECode_Success;
                }
        | RANGE_BEGIN query_kind COMMA a_graph_expr COMMA where_expr RANGE_END
                {
                  GQueryStmt* queryStmt = new GQueryStmt($2, $4, $6);
                  $$ = NewAst(NodeType::QueryStatement, queryStmt, nullptr, 0);
                  stm._errorCode = ECode_Success;
                };
query_kind: OP_QUERY COLON query_kind_expr { $$ = $3; }
        |   OP_QUERY COLON match_expr { $$ = $3; };
query_kind_expr: 
          KW_VERTEX { $$ = INIT_STRING_AST("vertex"); }
        | KW_EDGE { $$ = INIT_STRING_AST("edge"); }
        | KW_PATH { $$ = INIT_STRING_AST("path");}
        | a_graph_properties { $$ = $1; };
match_expr: //{->: 'alias'}
          RANGE_BEGIN a_match RANGE_END { $$ = $2; };
a_match:  a_relation_match
                {
                  $$ = $1;
                }
        | a_vertex_match COMMA a_relation_match COMMA a_vertex_match {};
a_relation_match: right_arrow COLON VAR_STRING
                {
                  struct GASTNode* to_value = INIT_STRING_AST($3);
                  GEdgeDeclaration* edge = new GEdgeDeclaration(nullptr, to_value, INIT_STRING_AST("->"));
                  $$ = NewAst(NodeType::EdgeDeclaration, edge, nullptr, 0);
                }
        | left_arrow COLON VAR_STRING
                {
                  struct GASTNode* to_value = INIT_STRING_AST($3);
                  GEdgeDeclaration* edge = new GEdgeDeclaration(nullptr, to_value, INIT_STRING_AST("<-"));
                  $$ = NewAst(NodeType::EdgeDeclaration, edge, nullptr, 0);
                };
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
                GArrayExpression* array = new GArrayExpression();
                array->addElement(INIT_STRING_AST($1));
                $$ = NewAst(NodeType::ArrayExpression, array, nullptr, 0);
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
                GArrayExpression* array = new GArrayExpression();
                array->addElement(INIT_STRING_AST($1));
                $$ = NewAst(NodeType::ArrayExpression, array, nullptr, 0);
              }
        | strings COMMA VAR_STRING
              {
                GArrayExpression* array = (GArrayExpression*)$1->_value;
                array->addElement(INIT_STRING_AST($3));
                $$ = $1;
              };
number: VAR_DECIMAL { $$ = INIT_NUMBER_AST($1); }
        | VAR_INTEGER { $$ = INIT_NUMBER_AST($1); };
a_graph_properties:
          graph_property { $$ = $1; }
        | LEFT_SQUARE graph_properties RIGHT_SQUARE { $$ = $2; };
graph_properties: 
          graph_property
              {
                GArrayExpression* array = new GArrayExpression();
                array->addElement($1);
                $$ = NewAst(NodeType::ArrayExpression, array, nullptr, 0);
              }
        | graph_properties COMMA graph_property
              {
                GArrayExpression* array = (GArrayExpression*)$1->_value;
                array->addElement($3);
                $$ = $1;
              };
graph_property:
          KW_VERTEX dot VAR_STRING {
            $$ = NewAst(NodeType::VariableDeclarator, INIT_STRING_AST($3), nullptr, 0);
          }
        | KW_EDGE dot VAR_STRING {}
        | KW_VERTEX dot function_call {}
        | KW_EDGE dot function_call
              {
                auto scope = INIT_STRING_AST("edge");
                $$ = NewAst(NodeType::VariableDeclarator, scope, $3, 1);
              };
vertex_list: LEFT_SQUARE vertexes RIGHT_SQUARE
              {
                $$ = $2;
              };
vertexes: vertex
              {
                GArrayExpression* vertexes = new GArrayExpression();
                vertexes->addElement($1);
                $$ = NewAst(NodeType::ArrayExpression, vertexes, nullptr, 0);
              }
        | vertexes COMMA vertex
              {
                GArrayExpression* vertexes = (GArrayExpression*)$1->_value;
                vertexes->addElement($3);
                $$ = $1;
              };
vertex: LEFT_SQUARE VAR_STRING RIGHT_SQUARE
              {
                GVertexDeclaration* decl = new GVertexDeclaration(INIT_STRING_AST($2), nullptr);
                $$ = NewAst(NodeType::VertexDeclaration, decl, nullptr, 0);
              }
        | LEFT_SQUARE VAR_STRING COMMA json RIGHT_SQUARE
              {
                GVertexDeclaration* decl = new GVertexDeclaration(INIT_STRING_AST($2), $4);
                $$ = NewAst(NodeType::VertexDeclaration, decl, nullptr, 0);
              };
edge_list: LEFT_SQUARE edges RIGHT_SQUARE {$$ = $2;};
edges: edge
              {
                GArrayExpression* edges = new GArrayExpression();
                edges->addElement($1);
                $$ = NewAst(NodeType::ArrayExpression, edges, nullptr, 0);
              }
        | edges COMMA edge
              {
                GArrayExpression* edges = (GArrayExpression*)$1->_value;
                edges->addElement($3);
                $$ = $1;
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
                free($1);
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
              GArrayExpression* props = new GArrayExpression();
              props->addElement($1);
              $$ = NewAst(NodeType::ArrayExpression, props, nullptr, 0);
            }
        | properties COMMA property
              {
                GArrayExpression* props = (GArrayExpression*)$1->_value;
                props->addElement($3);
                $$ = $1;
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
        | a_link_condition
              {
                $$ = $1;
              }
        | group COLON VAR_STRING
              {
                struct GASTNode* value = INIT_STRING_AST($3);
                GProperty* prop = new GProperty("group", value);
                $$ = NewAst(NodeType::Property, prop, nullptr, 0);
              };
array:    LEFT_SQUARE RIGHT_SQUARE { $$ = nullptr; }
        | LEFT_SQUARE values RIGHT_SQUARE
              {
                $$ = $2;
              };
values: value {
                GArrayExpression* values = new GArrayExpression();
                values->addElement($1);
                $$ = NewAst(NodeType::ArrayExpression, values, nullptr, 0);
              }
        | values COMMA value
              {
                GArrayExpression* values = (GArrayExpression*)$1->_value;
                values->addElement($3);
                $$ = $1;
              }
        | values COMMA KW_REST {};
a_link_condition: 
        | a_edge COLON a_value
              {
                GEdgeDeclaration* edge = new GEdgeDeclaration(nullptr, $3, $1);
                $$ = NewAst(NodeType::EdgeDeclaration, edge, nullptr, 0);
              };
a_edge:
        | right_arrow { $$ = INIT_STRING_AST("->");}
        | left_arrow { $$ = INIT_STRING_AST("<-"); }
        | KW_BIDIRECT_RELATION { $$ = INIT_STRING_AST("--"); };
a_value:
        | VAR_STRING { $$ = INIT_STRING_AST($1); }
        | VAR_DECIMAL { $$ = INIT_NUMBER_AST($1); }
        | VAR_INTEGER { $$ = INIT_NUMBER_AST($1); };
function_call:
        | VAR_STRING function_params
              {
                auto fname = INIT_STRING_AST($1);
                if ($2 == nullptr) {
                  $$ = NewAst(NodeType::CallExpression, fname, $2, 0);
                }
                else {
                  $$ = NewAst(NodeType::CallExpression, fname, $2, 1);
                }
              };
function_params:
        | PARAM_BEGIN PARAM_END { $$ = nullptr; }
        | PARAM_BEGIN STAR PARAM_END { $$ = INIT_STRING_AST("*"); }
        | PARAM_BEGIN string_list PARAM_END { $$ = $2; }
        ;
%%
