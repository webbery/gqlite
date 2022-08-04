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
#include "Type/Binary.h"
#include "base/lang/lang.h"
#include "base/lang/AST.h"

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
  float __f;
  char var_name[32];
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

}

%token RANGE_BEGIN RANGE_END COLON QUOTE COMMA LEFT_SQUARE RIGHT_SQUARE STAR CR PARAM_BEGIN PARAM_END
%token <var_name> VAR_HASH
%token <__f> VAR_DECIMAL
%token <__c> VAR_BASE64 VAR_STRING
%token <__int> VAR_INTEGER
%token <__int> VAR_DATETIME
%token KW_AST KW_ID KW_GRAPH KW_COMMIT
%token KW_CREATE KW_DROP KW_IN KW_REMOVE KW_UPSET left_arrow right_arrow KW_BIDIRECT_RELATION KW_REST KW_DELETE
%token OP_QUERY KW_INDEX OP_WHERE
%token group dump
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
%type <__node> value number right_value
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
%type <__node> creation dump_graph
%type <__node> upset_vertexes
%type <__node> upset_edges
%type <__node> a_simple_query
%type <__node> query_kind_expr a_match   a_relation_match  match_expr
%type <__node> query_kind
%type <__node> a_graph_properties graph_property graph_properties
%type <__node> a_edge
%type <__node> a_link_condition
%type <__node> a_value
%type <__node> a_group group_list groups
%type <__node> drop_graph
%type <__node> remove_vertexes
%type <__node> vertex_list vertexes string_list strings property_list edges edge_list

%start line_list

%%
line_list: line
          | line_list line
          | line error
          ;
    line: gql
          {
            stm._errorCode = stm.execAST($1);
            FreeAst($1);
          }
        | utility_cmd { stm._cmdtype = GQL_Util; }
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
        | dump_graph
          {
            GGQLExpression* expr = new GGQLExpression();
            $$ = NewAst(NodeType::GQLExpression, expr, $1, 1);
            stm._cmdtype = GQL_Util;
          }
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
            GGQLExpression* expr = new GGQLExpression(GGQLExpression::CMDType::SHOW_GRAPH_DETAIL, $3);
            free($3);
            auto ast = NewAst(NodeType::GQLExpression, expr, nullptr, 0);
            stm._errorCode = stm.execCommand(ast);
            FreeAst(ast);
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
              free($4);
              $$ = NewAst(NodeType::CreationStatement, createStmt, nullptr, 0);
              stm._errorCode = ECode_Success;
            }
        | RANGE_BEGIN KW_CREATE COLON VAR_STRING COMMA groups RANGE_END
            {
              GCreateStmt* createStmt = new GCreateStmt($4, $6);
              free($4);
              $$ = NewAst(NodeType::CreationStatement, createStmt, nullptr, 0);
              stm._errorCode = ECode_Success;
            }
        | RANGE_BEGIN KW_CREATE COLON VAR_STRING COMMA KW_INDEX COLON function_call RANGE_END
              {
                free($4);
              }
        | RANGE_BEGIN KW_CREATE COLON VAR_STRING COMMA groups COMMA KW_INDEX COLON string_list RANGE_END
              {
                GCreateStmt* createStmt = new GCreateStmt($4, $6, $10);
                free($4);
                $$ = NewAst(NodeType::CreationStatement, createStmt, nullptr, 0);
                stm._errorCode = ECode_Success;
              };
dump_graph: RANGE_BEGIN dump COLON VAR_STRING RANGE_END
              {
                GDumpStmt* stmt = new GDumpStmt($4);
                free($4);
                $$ = NewAst(NodeType::DumpStatement, stmt, nullptr, 0);
                stm._errorCode = ECode_Success;
              };
upset_vertexes: RANGE_BEGIN KW_UPSET COLON VAR_STRING COMMA KW_VERTEX COLON vertex_list RANGE_END
              {
                // struct YYLTYPE* ltype = &@8;
                // printf("upset_vertexes: %d, %d, %d, %d\n", ltype->first_line, ltype->first_column, ltype->last_line, ltype->last_column);
                GUpsetStmt* upsetStmt = new GUpsetStmt($4, $8);
                free($4);
                $$ = NewAst(NodeType::UpsetStatement, upsetStmt, nullptr, 0);
              };
remove_vertexes: RANGE_BEGIN KW_REMOVE COLON VAR_STRING COMMA KW_VERTEX COLON vertexes RANGE_END
              {
                GRemoveStmt* rmStmt = new GRemoveStmt($4, $8);
                free($4);
                $$ = NewAst(NodeType::RemoveStatement, rmStmt, nullptr, 0);
              };
upset_edges: RANGE_BEGIN KW_UPSET COLON VAR_STRING COMMA KW_EDGE COLON edge_list RANGE_END
              {
                GUpsetStmt* upsetStmt = new GUpsetStmt($4, $8);
                free($4);
                $$ = NewAst(NodeType::UpsetStatement, upsetStmt, nullptr, 0);
              };
drop_graph: RANGE_BEGIN KW_DROP COLON VAR_STRING RANGE_END
              {
                GDropStmt* dropStmt = new GDropStmt($4);
                free($4);
                $$ = NewAst(NodeType::DropStatement, dropStmt, nullptr, 0);
              };
groups: group COLON LEFT_SQUARE group_list RIGHT_SQUARE
              {
                $$ = $4;
              };
group_list: a_group
              {
                GArrayExpression* array = new GArrayExpression();
                array->addElement($1);
                $$ = NewAst(NodeType::ArrayExpression, array, nullptr, 0);
              }
        | group_list COMMA a_group
              {
                GArrayExpression* array = (GArrayExpression*)$1->_value;
                array->addElement($3);
                $$ = $1;
              };
a_group: VAR_STRING
              {
                GGroupStmt* stmt = new GGroupStmt($1);
                free($1);
                $$ = NewAst(NodeType::GroupStatement, stmt, nullptr, 0);
              }
        | RANGE_BEGIN VAR_STRING COLON string_list RANGE_END
              {
                GGroupStmt* stmt = new GGroupStmt($2, $4);
                free($2);
                $$ = NewAst(NodeType::GroupStatement, stmt, nullptr, 0);
                
              };
a_simple_query: 
           RANGE_BEGIN query_kind RANGE_END
                {
                  GQueryStmt* queryStmt = new GQueryStmt($2, nullptr, nullptr);
                  $$ = NewAst(NodeType::QueryStatement, queryStmt, nullptr, 0);
                  stm._errorCode = ECode_Success;
                }
        |  RANGE_BEGIN query_kind COMMA a_graph_expr RANGE_END
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
        |  KW_EDGE { $$ = INIT_STRING_AST("edge"); }
        |  VAR_STRING { $$ = INIT_STRING_AST($1); free($1); }
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
                  free($3);
                  GEdgeDeclaration* edge = new GEdgeDeclaration(nullptr, to_value, INIT_STRING_AST("->"));
                  $$ = NewAst(NodeType::EdgeDeclaration, edge, nullptr, 0);
                }
        | left_arrow COLON VAR_STRING
                {
                  struct GASTNode* to_value = INIT_STRING_AST($3);
                  free($3);
                  GEdgeDeclaration* edge = new GEdgeDeclaration(nullptr, to_value, INIT_STRING_AST("<-"));
                  $$ = NewAst(NodeType::EdgeDeclaration, edge, nullptr, 0);
                };
a_vertex_match: KW_VERTEX COLON VAR_STRING
                {
                  free($3);
                };
a_graph_expr:
          KW_IN COLON VAR_STRING
                {
                  $$ = INIT_STRING_AST($3);
                  free($3);
                };
where_expr: OP_WHERE COLON json { $$ = $3; };
string_list: VAR_STRING
                {
                  GArrayExpression* array = new GArrayExpression();
                  array->addElement(INIT_STRING_AST($1));
                  free($1);
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
                  free($1);
                  $$ = NewAst(NodeType::ArrayExpression, array, nullptr, 0);
                }
        | strings COMMA VAR_STRING
              {
                GArrayExpression* array = (GArrayExpression*)$1->_value;
                array->addElement(INIT_STRING_AST($3));
                free($3);
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
          KW_VERTEX dot VAR_STRING
              {
                GMemberExpression* expr = new GMemberExpression(INIT_STRING_AST("vertex"), INIT_STRING_AST($3));
                $$ = NewAst(NodeType::MemberExpression, expr, nullptr, 0);
                free($3);
              }
        | KW_EDGE dot VAR_STRING
              {
                GMemberExpression* expr = new GMemberExpression(INIT_STRING_AST("edge"), INIT_STRING_AST($3));
                $$ = NewAst(NodeType::MemberExpression, expr, nullptr, 0);
                free($3);
              }
        |  VAR_STRING dot VAR_STRING
              {
                GMemberExpression* expr = new GMemberExpression(INIT_STRING_AST($1), INIT_STRING_AST($3));
                $$ = NewAst(NodeType::MemberExpression, expr, nullptr, 0);
                free($3);
                free($1);
              }
        | KW_VERTEX dot function_call {}
        | KW_EDGE dot function_call {}
        | VAR_STRING dot function_call
              {
                auto scope = INIT_STRING_AST($1);
                $$ = NewAst(NodeType::VariableDeclarator, scope, $3, 1);
                free($1);
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
                free($2);
                $$ = NewAst(NodeType::VertexDeclaration, decl, nullptr, 0);
              }
        | LEFT_SQUARE VAR_STRING COMMA json RIGHT_SQUARE
              {
                GVertexDeclaration* decl = new GVertexDeclaration(INIT_STRING_AST($2), $4);
                free($2);
                $$ = NewAst(NodeType::VertexDeclaration, decl, nullptr, 0);
              }
        | LEFT_SQUARE VAR_INTEGER RIGHT_SQUARE
              {
                GVertexDeclaration* decl = new GVertexDeclaration(INIT_NUMBER_AST($2), nullptr);
                $$ = NewAst(NodeType::VertexDeclaration, decl, nullptr, 0);
              }
        | LEFT_SQUARE VAR_INTEGER COMMA json RIGHT_SQUARE
              {
                GVertexDeclaration* decl = new GVertexDeclaration(INIT_NUMBER_AST($2), $4);
                $$ = NewAst(NodeType::VertexDeclaration, decl, nullptr, 0);
              }
        | VAR_STRING
              {
                GVertexDeclaration* decl = new GVertexDeclaration(INIT_STRING_AST($1), nullptr);
                free($1);
                $$ = NewAst(NodeType::VertexDeclaration, decl, nullptr, 0);
              }
        | VAR_INTEGER
              {
                GVertexDeclaration* decl = new GVertexDeclaration(INIT_NUMBER_AST($1), nullptr);
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
                free($2);
                struct GASTNode* to_value = INIT_STRING_AST($6);
                free($6);
                GEdgeDeclaration* edge = new GEdgeDeclaration(from_value, to_value, $4);
                $$ = NewAst(NodeType::EdgeDeclaration, edge, nullptr, 0);
              }
        | LEFT_SQUARE VAR_STRING RIGHT_SQUARE
              {
                struct GASTNode* from_value = INIT_STRING_AST($2);
                free($2);
                GEdgeDeclaration* edge = new GEdgeDeclaration(from_value, from_value, INIT_STRING_AST("--"));
                $$ = NewAst(NodeType::EdgeDeclaration, edge, nullptr, 0);
              };
json: value { $$ = $1; };
value: object { $$ = $1; }
        | array { $$ = $1; };
right_value: value { $$ = $1; }
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
                $$ = INIT_STRING_AST($1);
                free($1);
              };
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
property: VAR_STRING COLON right_value
              {
                GProperty* prop = new GProperty($1, $3);
                free($1);
                $$ = NewAst(NodeType::Property, prop, nullptr, 0);
              }
        | KW_ID COLON VAR_STRING
              {
                struct GASTNode* value = INIT_STRING_AST($3);
                free($3);
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
                free($3);
                GProperty* prop = new GProperty("group", value);
                $$ = NewAst(NodeType::Property, prop, nullptr, 0);
              };
array:    LEFT_SQUARE RIGHT_SQUARE { $$ = nullptr; }
        | LEFT_SQUARE values RIGHT_SQUARE
              {
                $$ = $2;
              };
values: right_value {
                GArrayExpression* values = new GArrayExpression();
                values->addElement($1);
                $$ = NewAst(NodeType::ArrayExpression, values, nullptr, 0);
              }
        | values COMMA right_value
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
        | VAR_STRING
              {
                $$ = INIT_STRING_AST($1);
                free($1);
              }
        | VAR_DECIMAL { $$ = INIT_NUMBER_AST($1); }
        | VAR_INTEGER { $$ = INIT_NUMBER_AST($1); };
function_call:
        | VAR_STRING function_params
              {
                auto fname = INIT_STRING_AST($1);
                free($1);
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
