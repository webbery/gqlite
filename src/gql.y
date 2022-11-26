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

%code {
#define YY_DECL \
       int yylex(YYSTYPE* yylval_param, YYLTYPE* yylloc_param, void* yyscanner, GVirtualEngine& stm)

void yyerror(YYLTYPE* yyllocp, yyscan_t unused, GVirtualEngine& stm, const char* msg) {
  std::string err_index;
  if (stm._errIndx) {
    std::string err_offset(stm._errIndx - 1, ' ');
    err_index += err_offset + "~";
  }
  // printf("\033[22;0m%s\n%s\n",
  //   stm.gql().c_str(), err_index.c_str());
    printf("\033[22;31mError:\t%s:\033[22;0m\n\t%s\n\t%s\n",
      msg, stm.gql().c_str(), err_index.c_str());
}

struct GASTNode* INIT_STRING_AST(const char* key) {
  size_t len = strlen(key);
  // void* s = malloc(len);
  // memcpy(s, key, len);
  // printf("|-> %s\n", key);
  GLiteral* str = new GLiteralString(key, len);
  return NewAst(NodeType::Literal, str, nullptr, 0);
}

template<typename T>
struct GASTNode* INIT_NUMBER_AST(T v) {
  GLiteral* number = new GLiteralNumber(v);
  return NewAst(NodeType::Literal, number, nullptr, 0);
}

struct GASTNode* INIT_NUMBER_AST(double v, AttributeKind kind) {
  switch (kind) {
  case AttributeKind::Integer:
    if (v <= std::numeric_limits<int>::max()) {
      return INIT_NUMBER_AST((int)v);
    }
    else if (v <= std::numeric_limits<long>::max()) {
      return INIT_NUMBER_AST((long)v);
    }
    else if (v > 0 && v <= std::numeric_limits<uint64_t>::max()){
      return INIT_NUMBER_AST((uint64_t)v);
    }
    else { // use double
      return INIT_NUMBER_AST(v);
    }
  default:
    return INIT_NUMBER_AST(v);
  }
}
} // %code

%lex-param {GVirtualEngine& stm}
%parse-param {GVirtualEngine& stm}
%union {
  struct GASTNode* node;
  double __f;
  char var_name[32];
  char* __c;
  size_t __offset;
  time_t __datetime;
  int32_t __int;
  nlohmann::json* __json;
}

%start line_list

%token <var_name> VAR_HASH
%token <__f> VAR_DECIMAL
%token <__c> VAR_BASE64 LITERAL_STRING VAR_NAME LITERAL_PATH
%token <__f> VAR_INTEGER
%token <__datetime> VAR_DATETIME
%token <node> KW_VERTEX KW_EDGE
%token RANGE_BEGIN RANGE_END COLON QUOTE COMMA LEFT_SQUARE RIGHT_SQUARE STAR CR PARAM_BEGIN PARAM_END SEMICOLON
%token KW_AST KW_ID KW_GRAPH KW_COMMIT
%token KW_CREATE KW_DROP KW_IN KW_REMOVE KW_UPSET left_arrow right_arrow KW_BIDIRECT_RELATION KW_REST KW_DELETE
%token OP_QUERY KW_INDEX OP_WHERE OP_GEOMETRY neighbor
%token group dump import
%token CMD_SHOW 
%token OP_GREAT_THAN OP_LESS_THAN OP_GREAT_THAN_EQUAL OP_LESS_THAN_EQUAL equal AND OR OP_NEAR
%token FN_COUNT
%token dot
%token limit profile

%type <var_name> a_edge
%type <node> a_graph_expr
%type <node> condition_json normal_json
%type <node> condition_value normal_value number right_value simple_value geometry_condition range_comparable datetime_comparable
%type <node> condition_values normal_values simple_values
%type <node> normal_object condition_object
%type <node> condition_array normal_array a_vector number_list
%type <node> normal_properties condition_properties
%type <node> where_expr a_walk vertex_start_walk edge_start_walk a_simple_graph
%type <node> function_call function_params
%type <node> normal_property condition_property
%type <node> vertex 
%type <node> gql
%type <node> creation dump_graph
%type <node> upset_vertexes
%type <node> upset_edges edge_pattern connection
%type <node> a_simple_query
%type <node> query_kind_expr a_match match_expr
%type <node> query_kind
%type <node> a_graph_properties graph_property graph_properties
%type <node> a_link_condition
%type <node> a_value
%type <node> a_group group_list groups
%type <node> drop_graph
%type <node> remove_vertexes
%type <node> vertex_list vertexes string_list strings property_list links link

%%
line_list: line_list line SEMICOLON {}
          | line SEMICOLON {}
          /* | error SEMICOLON {} */
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
        | CMD_SHOW KW_GRAPH LITERAL_STRING
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
        | profile gql {}
        | import LITERAL_PATH
          {
            free($2);
            stm._cmdtype = GQL_Util;
          }
        ;
creation: RANGE_BEGIN KW_CREATE COLON LITERAL_STRING COMMA groups RANGE_END
            {
              GCreateStmt* createStmt = new GCreateStmt($4, $6);
              free($4);
              $$ = NewAst(NodeType::CreationStatement, createStmt, nullptr, 0);
              stm._errorCode = ECode_Success;
            }
        | RANGE_BEGIN KW_CREATE COLON LITERAL_STRING COMMA KW_INDEX COLON function_call RANGE_END
              {
                free($4);
              }
        ;
dump_graph: RANGE_BEGIN dump COLON LITERAL_STRING RANGE_END
              {
                GDumpStmt* stmt = new GDumpStmt($4);
                free($4);
                $$ = NewAst(NodeType::DumpStatement, stmt, nullptr, 0);
                stm._errorCode = ECode_Success;
              };
upset_vertexes: RANGE_BEGIN KW_UPSET COLON LITERAL_STRING COMMA KW_VERTEX COLON vertex_list RANGE_END
              {
                // struct YYLTYPE* ltype = &@8;
                // printf("upset_vertexes: %d, %d, %d, %d\n", ltype->first_line, ltype->first_column, ltype->last_line, ltype->last_column);
                GUpsetStmt* upsetStmt = new GUpsetStmt($4, $8);
                free($4);
                $$ = NewAst(NodeType::UpsetStatement, upsetStmt, nullptr, 0);
              }
        | error RANGE_END
              {
                printf("\033[22;31mERROR:\t%s:\033[22;0m\n",
                  "should you use upset edge? input format is {upset: [vertex, edge, vertex], edge: ...}\n");
                yyerrok;
                stm._errorCode = GQL_GRAMMAR_OBJ_FAIL;
                YYABORT;
              };
remove_vertexes: RANGE_BEGIN KW_REMOVE COLON LITERAL_STRING COMMA KW_VERTEX COLON vertex_list RANGE_END
              {
                GRemoveStmt* rmStmt = new GRemoveStmt($4, $8);
                free($4);
                $$ = NewAst(NodeType::RemoveStatement, rmStmt, nullptr, 0);
              };
upset_edges: RANGE_BEGIN KW_UPSET COLON LITERAL_STRING COMMA KW_EDGE COLON LEFT_SQUARE links RIGHT_SQUARE RANGE_END
              {
                GUpsetStmt* upsetStmt = new GUpsetStmt($4, $9);
                free($4);
                $$ = NewAst(NodeType::UpsetStatement, upsetStmt, nullptr, 0);
              };
drop_graph: RANGE_BEGIN KW_DROP COLON LITERAL_STRING RANGE_END
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
a_group: LITERAL_STRING
              {
                GGroupStmt* stmt = new GGroupStmt($1);
                free($1);
                $$ = NewAst(NodeType::GroupStatement, stmt, nullptr, 0);
              }
        | RANGE_BEGIN VAR_NAME COLON string_list RANGE_END
              {
                GGroupStmt* stmt = new GGroupStmt($2, $4);
                free($2);
                $$ = NewAst(NodeType::GroupStatement, stmt, nullptr, 0);
              }
        | RANGE_BEGIN VAR_NAME COLON string_list COMMA KW_INDEX COLON string_list RANGE_END
              {
                GGroupStmt* stmt = new GGroupStmt($2, $4, $8);
                free($2);
                $$ = NewAst(NodeType::GroupStatement, stmt, nullptr, 0);
              }
        ;
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
a_simple_graph: LEFT_SQUARE a_walk RIGHT_SQUARE
                {
                  $$ = $2;
                  // GArrayExpression* array = new GArrayExpression();
                  // array->addElement($1);
                  // $$ = NewAst(NodeType::ArrayExpression, array, nullptr, 0);
                }
        | a_simple_graph COMMA LEFT_SQUARE a_walk RIGHT_SQUARE
                {
                  // $1->addElement($4);
                  // $$ = $1;
                };
a_walk: vertex_start_walk
                {
                  $$ = $1;
                }
        | vertex_start_walk COMMA vertex
                {
                  ((GWalkDeclaration*)($1->_value))->add($3, true);
                }
        | edge_start_walk
                {
                  $$ = $1;
                }
        | edge_start_walk COMMA vertex
                {
                  // $1->add($3, )
                };
vertex_start_walk: vertex COMMA a_link_condition
                {
                  GWalkDeclaration* walkDecl = new GWalkDeclaration();
                  walkDecl->add($1, true);
                  walkDecl->add($3, false);
                  $$ = NewAst(NodeType::WalkDeclaration, walkDecl, nullptr, 0);
                }
        | vertex_start_walk COMMA vertex COMMA a_link_condition
                {
                  ((GWalkDeclaration*)($1->_value))->add($3, true);
                  ((GWalkDeclaration*)($1->_value))->add($5, false);
                  $$ = $1;
                };
edge_start_walk: a_link_condition COMMA vertex
                {
                  GWalkDeclaration* walkDecl = new GWalkDeclaration();
                  walkDecl->add($1, true);
                  walkDecl->add($3, false);
                  $$ = NewAst(NodeType::WalkDeclaration, walkDecl, nullptr, 0);
                }
        | edge_start_walk COMMA a_link_condition COMMA vertex {};
query_kind: OP_QUERY COLON query_kind_expr { $$ = $3; }
        |   OP_QUERY COLON match_expr { $$ = $3; };
query_kind_expr: 
          KW_EDGE { $$ = INIT_STRING_AST("edge"); }
        |  LITERAL_STRING { $$ = INIT_STRING_AST($1); free($1); }
        | a_graph_properties { $$ = $1; };
match_expr: //{->: 'alias'}
          RANGE_BEGIN a_match RANGE_END { $$ = $2; };
a_match:  a_simple_graph { $$ = $1; };
a_graph_expr:
          KW_IN COLON LITERAL_STRING
                {
                  $$ = INIT_STRING_AST($3);
                  free($3);
                };
where_expr: OP_WHERE COLON condition_json { $$ = $3; }
      |     OP_WHERE COLON a_match {};
string_list: LITERAL_STRING
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
        | normal_property {};
// property_list: STAR { $$ = nullptr; }
//         | string_list { $$ = $1; };
strings:  LITERAL_STRING
                {
                  GArrayExpression* array = new GArrayExpression();
                  array->addElement(INIT_STRING_AST($1));
                  free($1);
                  $$ = NewAst(NodeType::ArrayExpression, array, nullptr, 0);
                }
        | strings COMMA LITERAL_STRING
              {
                GArrayExpression* array = (GArrayExpression*)$1->_value;
                array->addElement(INIT_STRING_AST($3));
                free($3);
                $$ = $1;
              };
number: VAR_DECIMAL { $$ = INIT_NUMBER_AST($1, AttributeKind::Number); }
        | VAR_INTEGER { $$ = INIT_NUMBER_AST($1, AttributeKind::Integer); };
a_graph_properties:
          graph_property { $$ = $1; }
        | LEFT_SQUARE graph_properties RIGHT_SQUARE { $$ = $2; }
        | error RIGHT_SQUARE
          {
            printf("\033[22;31mERROR:\t%s:\033[22;0m\n",
              "input object is not a correct json");
            yyerrok;
            stm._errorCode = GQL_GRAMMAR_OBJ_FAIL;
            YYABORT;
          }
        ;
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
          KW_VERTEX dot VAR_NAME
              {
                GMemberExpression* expr = new GMemberExpression(INIT_STRING_AST("vertex"), INIT_STRING_AST($3));
                $$ = NewAst(NodeType::MemberExpression, expr, nullptr, 0);
                free($3);
              }
        | KW_EDGE dot VAR_NAME
              {
                GMemberExpression* expr = new GMemberExpression(INIT_STRING_AST("edge"), INIT_STRING_AST($3));
                $$ = NewAst(NodeType::MemberExpression, expr, nullptr, 0);
                free($3);
              }
        |  VAR_NAME dot VAR_NAME
              {
                GMemberExpression* expr = new GMemberExpression(INIT_STRING_AST($1), INIT_STRING_AST($3));
                $$ = NewAst(NodeType::MemberExpression, expr, nullptr, 0);
                free($3);
                free($1);
              }
        | KW_VERTEX dot function_call {}
        | KW_EDGE dot function_call {}
        | VAR_NAME dot function_call
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
vertex: LEFT_SQUARE LITERAL_STRING COMMA normal_json RIGHT_SQUARE
              {
                GVertexDeclaration* decl = new GVertexDeclaration(INIT_STRING_AST($2), $4);
                free($2);
                $$ = NewAst(NodeType::VertexDeclaration, decl, nullptr, 0);
              }
        | LEFT_SQUARE VAR_INTEGER RIGHT_SQUARE
              {
                GVertexDeclaration* decl = new GVertexDeclaration(INIT_NUMBER_AST($2, AttributeKind::Integer), nullptr);
                $$ = NewAst(NodeType::VertexDeclaration, decl, nullptr, 0);
              }
        | LEFT_SQUARE VAR_INTEGER COMMA normal_json RIGHT_SQUARE
              {
                GVertexDeclaration* decl = new GVertexDeclaration(INIT_NUMBER_AST($2, AttributeKind::Integer), $4);
                $$ = NewAst(NodeType::VertexDeclaration, decl, nullptr, 0);
              }
        | LITERAL_STRING
              {
                GVertexDeclaration* decl = new GVertexDeclaration(INIT_STRING_AST($1), nullptr);
                free($1);
                $$ = NewAst(NodeType::VertexDeclaration, decl, nullptr, 0);
              }
        | VAR_INTEGER
              {
                GVertexDeclaration* decl = new GVertexDeclaration(INIT_NUMBER_AST($1, AttributeKind::Integer), nullptr);
                $$ = NewAst(NodeType::VertexDeclaration, decl, nullptr, 0);
              };
links: link
              {
                GArrayExpression* edges = new GArrayExpression();
                edges->addElement($1);
                $$ = NewAst(NodeType::ArrayExpression, edges, nullptr, 0);
              }
        | links COMMA link
              {
                GArrayExpression* edges = (GArrayExpression*)$1->_value;
                edges->addElement($3);
                $$ = $1;
              };
link: LEFT_SQUARE LITERAL_STRING COMMA connection COMMA LITERAL_STRING RIGHT_SQUARE
              {
                struct GASTNode* from_value = INIT_STRING_AST($2);
                free($2);
                struct GASTNode* to_value = INIT_STRING_AST($6);
                free($6);
                GEdgeDeclaration* edge = new GEdgeDeclaration($4, from_value, to_value);
                $$ = NewAst(NodeType::EdgeDeclaration, edge, nullptr, 0);
              }
        | LEFT_SQUARE LITERAL_STRING RIGHT_SQUARE
              {
                struct GASTNode* value = INIT_STRING_AST($2);
                free($2);
                GEdgeDeclaration* edge = new GEdgeDeclaration("--", value);
                $$ = NewAst(NodeType::EdgeDeclaration, edge, nullptr, 0);
              }
        | LEFT_SQUARE VAR_INTEGER COMMA connection COMMA VAR_INTEGER RIGHT_SQUARE
              {
                GEdgeDeclaration* edge = new GEdgeDeclaration($4, INIT_NUMBER_AST($2, AttributeKind::Integer), INIT_NUMBER_AST($6, AttributeKind::Integer));
                $$ = NewAst(NodeType::EdgeDeclaration, edge, nullptr, 0);
              }
        | LEFT_SQUARE VAR_INTEGER RIGHT_SQUARE
              {
                struct GASTNode* value = INIT_NUMBER_AST($2, AttributeKind::Integer);
                GEdgeDeclaration* edge = new GEdgeDeclaration("--", value);
                $$ = NewAst(NodeType::EdgeDeclaration, edge, nullptr, 0);
              };
connection: a_link_condition { $$ = $1;}
        | a_edge  { $$ = INIT_STRING_AST($1); };
a_vector: LEFT_SQUARE number_list RIGHT_SQUARE { $$ = $2; };
number_list: number
              {
                GArrayExpression* elemts = new GArrayExpression();
                elemts->addElement($1);
                $$ = NewAst(NodeType::ArrayExpression, elemts, nullptr, 0);
              }
        | number_list COMMA number
              {
                GArrayExpression* elemts = (GArrayExpression*)$1->_value;
                elemts->addElement($3);
                $$ = $1;
              };
normal_json: normal_value { $$ = $1; };
condition_json: condition_value { $$ = $1; };
normal_value: normal_object { $$ = $1; }
        | normal_array { $$ = $1; };
condition_value: condition_object { $$ = $1; }
        | condition_array { $$ = $1; };
right_value: condition_value { $$ = $1; }
        | simple_value { $$ = $1; };
simple_value: VAR_BASE64
              {
                GLiteralBinary* bin = new GLiteralBinary($1, "b64");
                free($1);
                $$ = NewAst(NodeType::Literal, bin, nullptr, 0);
              }
        | LITERAL_STRING
              {
                $$ = INIT_STRING_AST($1);
                free($1);
              }
        | VAR_DATETIME
              {
                GLiteralDatetime* dt = new GLiteralDatetime($1);
                $$ = NewAst(NodeType::Literal, dt, nullptr, 0);
              }
        | number
              {
                $$ = $1;
              };
normal_object: RANGE_BEGIN normal_properties RANGE_END
            {
              $$ = NewAst(NodeType::ObjectExpression, $2, nullptr, 0);
            };
condition_object: RANGE_BEGIN condition_properties RANGE_END
            {
              $$ = NewAst(NodeType::ObjectExpression, $2, nullptr, 0);
            }
        | error RANGE_END
            {
              printf("\033[22;31mERROR:\t%s:\033[22;0m\n",
                "input object is not a correct property");
              yyerrok;
              stm._errorCode = GQL_GRAMMAR_OBJ_FAIL;
              YYABORT;
            }
        ;
normal_properties: normal_property {
              GArrayExpression* props = new GArrayExpression();
              props->addElement($1);
              $$ = NewAst(NodeType::ArrayExpression, props, nullptr, 0);
            }
        | normal_properties COMMA normal_property
              {
                GArrayExpression* props = (GArrayExpression*)$1->_value;
                props->addElement($3);
                $$ = $1;
              };
condition_properties: condition_property
              {
                GArrayExpression* props = new GArrayExpression();
                props->addElement($1);
                $$ = NewAst(NodeType::ArrayExpression, props, nullptr, 0);
              }
        | condition_properties COMMA condition_property
              {
                GArrayExpression* props = (GArrayExpression*)$1->_value;
                props->addElement($3);
                $$ = $1;
              };
normal_property: VAR_NAME COLON simple_value
              {
                GProperty* prop = new GProperty($1, $3);
                free($1);
                $$ = NewAst(NodeType::Property, prop, nullptr, 0);
              }
        | VAR_NAME COLON normal_value
              {
                GProperty* prop = new GProperty($1, $3);
                free($1);
                $$ = NewAst(NodeType::Property, prop, nullptr, 0);
              };
condition_property: VAR_NAME COLON right_value
              {
                GProperty* prop = new GProperty($1, $3);
                free($1);
                $$ = NewAst(NodeType::Property, prop, nullptr, 0);
              }
        | datetime_comparable { $$ = $1;}
        | range_comparable { $$ = $1;}
        | KW_ID COLON LITERAL_STRING
              {
                struct GASTNode* value = INIT_STRING_AST($3);
                free($3);
                GProperty* prop = new GProperty("id", value);
                $$ = NewAst(NodeType::Property, prop, nullptr, 0);
              }
        | AND COLON condition_array
              {
                GProperty* prop = new GProperty("and", $3);
                $$ = NewAst(NodeType::BinaryExpression, prop, nullptr, 0);
              }
        | OR COLON condition_array
              {
                GProperty* prop = new GProperty("or", $3);
                $$ = NewAst(NodeType::BinaryExpression, prop, nullptr, 0);
              }
        | a_walk
              {
                $$ = $1;
              }
        | group COLON LITERAL_STRING
              {
                struct GASTNode* value = INIT_STRING_AST($3);
                free($3);
                GProperty* prop = new GProperty("group", value);
                $$ = NewAst(NodeType::Property, prop, nullptr, 0);
              }
        | OP_NEAR COLON RANGE_BEGIN geometry_condition RANGE_END
              {
                GObjectFunction* obj = (GObjectFunction*)($4->_value);
                obj->setFunctionName("__near__", "__global__");
                GProperty* prop = new GProperty("near", $4);
                $$ = NewAst(NodeType::BinaryExpression, prop, nullptr, 0);
              };
range_comparable: OP_GREAT_THAN_EQUAL COLON number
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
              };
datetime_comparable: OP_GREAT_THAN_EQUAL COLON VAR_DATETIME
              {
                GLiteralDatetime* dt = new GLiteralDatetime($3);
                GProperty* prop = new GProperty("gte", NewAst(NodeType::Literal, dt, nullptr, 0));
                $$ = NewAst(NodeType::BinaryExpression, prop, nullptr, 0);
              }
        | OP_LESS_THAN_EQUAL COLON VAR_DATETIME
              {
                GLiteralDatetime* dt = new GLiteralDatetime($3);
                GProperty* prop = new GProperty("lte", NewAst(NodeType::Literal, dt, nullptr, 0));
                $$ = NewAst(NodeType::BinaryExpression, prop, nullptr, 0);
              }
        | OP_GREAT_THAN COLON VAR_DATETIME
              {
                GLiteralDatetime* dt = new GLiteralDatetime($3);
                GProperty* prop = new GProperty("gt", NewAst(NodeType::Literal, dt, nullptr, 0));
                $$ = NewAst(NodeType::BinaryExpression, prop, nullptr, 0);
              }
        | OP_LESS_THAN COLON VAR_DATETIME
              {
                GLiteralDatetime* dt = new GLiteralDatetime($3);
                GProperty* prop = new GProperty("lt", NewAst(NodeType::Literal, dt, nullptr, 0));
                $$ = NewAst(NodeType::BinaryExpression, prop, nullptr, 0);
              };
geometry_condition: OP_GEOMETRY COLON a_vector COMMA range_comparable
              {
                GObjectFunction* obj = new GObjectFunction();
                obj->addFunctionParam($3);
                obj->addFunctionParam($5);
                $$ = NewAst(NodeType::CallExpression, obj, nullptr, 0);
              }
        | range_comparable COMMA OP_GEOMETRY COLON a_vector
              {
                GObjectFunction* obj = new GObjectFunction();
                obj->addFunctionParam($5);
                obj->addFunctionParam($1);
                $$ = NewAst(NodeType::CallExpression, obj, nullptr, 0);
              };
condition_array: LEFT_SQUARE RIGHT_SQUARE { $$ = nullptr; }
        | LEFT_SQUARE condition_values RIGHT_SQUARE
              {
                $$ = $2;
              }
        | error RIGHT_SQUARE
              {
                printf("\033[22;31mDetail:\t%s:\033[22;0m\n",
                  "condition array is not a correct array");
                stm._errorCode = GQL_GRAMMAR_ARRAY_FAIL;
                $$ = nullptr;
              };
normal_array:    LEFT_SQUARE RIGHT_SQUARE { $$ = nullptr; }
        | LEFT_SQUARE normal_values RIGHT_SQUARE
              {
                $$ = $2;
              }
        | LEFT_SQUARE simple_values RIGHT_SQUARE
              {
                $$ = $2;
              }
        | error RIGHT_SQUARE
              {
                printf("\033[22;31mDetail:\t%s:\033[22;0m\n",
                  "array is not a correct array");
                stm._errorCode = GQL_GRAMMAR_ARRAY_FAIL;
                $$ = nullptr;
              }
        ;
normal_values: normal_value
              {
                GArrayExpression* values = new GArrayExpression();
                values->addElement($1);
                $$ = NewAst(NodeType::ArrayExpression, values, nullptr, 0);
              }
        | normal_values COMMA normal_value
              {
                GArrayExpression* values = (GArrayExpression*)$1->_value;
                values->addElement($3);
                $$ = $1;
              };
simple_values: simple_value
              {
                GArrayExpression* values = new GArrayExpression();
                values->addElement($1);
                $$ = NewAst(NodeType::ArrayExpression, values, nullptr, 0);
              }
        | simple_values COMMA simple_value
              {
                GArrayExpression* values = (GArrayExpression*)$1->_value;
                values->addElement($3);
                $$ = $1;
              };
condition_values: right_value {
                GArrayExpression* values = new GArrayExpression();
                values->addElement($1);
                $$ = NewAst(NodeType::ArrayExpression, values, nullptr, 0);
              }
        | condition_values COMMA right_value
              {
                GArrayExpression* values = (GArrayExpression*)$1->_value;
                values->addElement($3);
                $$ = $1;
              }
        | condition_values COMMA KW_REST {};
a_link_condition: a_edge COLON a_value
              {
                GEdgeDeclaration* edge = new GEdgeDeclaration($1, $3);
                $$ = NewAst(NodeType::EdgeDeclaration, edge, nullptr, 0);
              }
        | a_edge COLON normal_json
              {
                GEdgeDeclaration* edge = new GEdgeDeclaration($1, $3);
                $$ = NewAst(NodeType::EdgeDeclaration, edge, nullptr, 0);
              };
a_edge:   right_arrow { memcpy(&$$, "->", 3);}
        | left_arrow { memcpy(&$$, "<-", 3); }
        | KW_BIDIRECT_RELATION { memcpy(&$$, "--", 3); };
a_value:  LITERAL_STRING
              {
                $$ = INIT_STRING_AST($1);
                free($1);
              }
        | VAR_DECIMAL { $$ = INIT_NUMBER_AST($1, AttributeKind::Number); }
        | VAR_INTEGER { $$ = INIT_NUMBER_AST($1, AttributeKind::Integer); };
function_call:
        | VAR_NAME function_params
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
