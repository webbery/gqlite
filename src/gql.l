%{
#include <stdio.h>
// #define YY_USER_ACTION                                             \
//     yylloc->first_line = yylloc->last_line;                          \
//     yylloc->first_column = yylloc->last_column;                      \
//     if (yylloc->last_line == yylineno)                               \
//         yylloc->last_column += yyleng;                                 \
//     else {                                                           \
//         yylloc->last_line = yylineno;                                  \
//         yylloc->last_column = yytext + yyleng - strrchr(yytext, '\n'); \
//     }
// %option prefix="gql_yy"
%}

%option reentrant bison-bridge bison-locations
%option noinput nounput noyywrap never-interactive

%%
^\/\/.*             { stm._errIndx += yyleng;}
"("                 { stm._errIndx += 1; return PARAM_BEGIN;};
")"                 { stm._errIndx += 1; return PARAM_END;};
"{"                 { stm._errIndx += 1; return RANGE_BEGIN;};
"}"                 { stm._errIndx += 1; return RANGE_END;};
"\""                { stm._errIndx += 1; };
"'*'"               { stm._errIndx += 3;return STAR; };
[ ]*                { stm._errIndx += yyleng; };
"\["                { stm._errIndx += 1; return LEFT_SQUARE; };
"\]"                { stm._errIndx += 1; return RIGHT_SQUARE; };
":"                 { stm._errIndx += 1; return COLON;};
","                 { stm._errIndx += 1; return COMMA;};
h'' {
                        memcpy(yylval->var_name, yytext + 1, yyleng - 2);
                        yylval->var_name[yyleng - 2] = '\0';
                        stm._errIndx += 1;
                        return VAR_HASH;
                    };
b64'([A-Za-z0-9+/]{4})*([A-Za-z0-9+\/]{2}==|[A-Za-z0-9+\/]{3}=)?' {
                        yylval->__c = (char*)malloc(yyleng - 5);
                        memcpy(yylval->__c, yytext + 4, yyleng - 6);
                        yylval->__c[yyleng - 6] = '\0';
                        stm._errIndx += yyleng;
                        return VAR_BASE64;
                    };
d[0-9]{10}$         {
                        memcpy(yylval->var_name, yytext + 1, yyleng - 2);
                        yylval->var_name[yyleng - 2] = '\0';
                        yylval->__datetime = strtoll(yylval->var_name, nullptr, 10);
                        stm._errIndx += yyleng;
                        return VAR_DATETIME;
                    };
'[a-zA-Z_]+[\w]*'    {
                        yylval->__c = (char*)malloc(yyleng - 1);
                        memcpy(yylval->__c, yytext + 1, yyleng - 2);
                        yylval->__c[yyleng - 2] = '\0';
                        stm._errIndx += yyleng;
                        return VAR_STRING;
                    };
'[a-zA-Z]+[\w]*\.[a-zA-Z]+' {
                        yylval->__c = (char*)malloc(yyleng - 1);
                        memcpy(yylval->__c, yytext + 1, yyleng - 2);
                        yylval->__c[yyleng - 2] = '\0';
                        stm._errIndx += yyleng;
                        return VAR_STRING;
                    };
'(\\.|[^'\\])*?'    {
                        yylval->__c = (char*)malloc(yyleng - 1);
                        memcpy(yylval->__c, yytext + 1, yyleng - 2);
                        yylval->__c[yyleng - 2] = '\0';
                        stm._errIndx += yyleng;
                        return VAR_STRING;
                    };
"extension"         {stm._errIndx += yyleng;};
"show"              {stm._errIndx += yyleng; return CMD_SHOW;};
"graph"             {stm._errIndx += yyleng; return KW_GRAPH;};
"walk"              {stm._errIndx += yyleng; };
"group"             {stm._errIndx += yyleng; return group;};
"order"             {stm._errIndx += yyleng;}
"start"             {stm._errIndx += yyleng;};
"with"              {stm._errIndx += yyleng;};
"id"                {stm._errIndx += yyleng; return KW_ID;};
"use"               { stm._errIndx += yyleng; };
create              {stm._errIndx += yyleng; return KW_CREATE;};
"drop"              { stm._errIndx += yyleng; return KW_DROP;};
"remove"            { stm._errIndx += yyleng; return KW_REMOVE;};
"delete"            { stm._errIndx += yyleng; return KW_DELETE;};
"upset"             { stm._errIndx += yyleng; return KW_UPSET;};
"vertex"            { stm._errIndx += yyleng; return KW_VERTEX;};
"edge"              { stm._errIndx += yyleng; return KW_EDGE;};
"ast"               { stm._errIndx += yyleng; return KW_AST;};
"query"             { stm._errIndx += yyleng; return OP_QUERY;};
"where"             { stm._errIndx += yyleng; return OP_WHERE;};
"index"             { stm._errIndx += yyleng; return KW_INDEX;};
"path"              { stm._errIndx += yyleng; return KW_PATH;};
"commit"            { stm._errIndx += yyleng; return KW_COMMIT;};
"limit"             { stm._errIndx += yyleng; return limit;};
"profile"           { stm._errIndx += yyleng; return profile;};
(\bf)unction        { stm._errIndx += yyleng;};
(\bi)f              { stm._errIndx += yyleng;};
"else"              { stm._errIndx += yyleng;};
"return"            { stm._errIndx += yyleng;};
"neighbor"          { stm._errIndx += yyleng;};
"$gte"              { stm._errIndx += yyleng; return OP_GREAT_THAN_EQUAL;};
"$lte"              { stm._errIndx += yyleng; return OP_LESS_THAN_EQUAL;};
"$gt"               { stm._errIndx += yyleng; return OP_GREAT_THAN;};
"$lt"               { stm._errIndx += yyleng; return OP_LESS_THAN;};
"$and"              { stm._errIndx += yyleng; return OP_AND;};
"$or"               { stm._errIndx += yyleng; return OP_OR;};
"<-"                { stm._errIndx += yyleng; return left_arrow;};
"->"                { stm._errIndx += yyleng; return right_arrow;};
"--"                { stm._errIndx += yyleng; return KW_BIDIRECT_RELATION;};
"in"                { stm._errIndx += yyleng; return KW_IN;};
"..."               { stm._errIndx += yyleng; return KW_REST;};
"."                 { stm._errIndx += 1; return dot;};
"+"                 {stm._errIndx += yyleng;};
"-"                 {stm._errIndx += yyleng;};
"*"                 {stm._errIndx += yyleng;};
"/"                 {stm._errIndx += yyleng;};
"\n"                {stm._errIndx += yyleng;};
[0-9]+              {
                        std::string s(yytext, yyleng);
                        yylval->__int = std::stoi(s);
                        stm._errIndx += yyleng;
                        return VAR_INTEGER;
                    };
[0-9]+.[0-9]+       {
                        std::string s(yytext, yyleng);
                        yylval->__f = std::stof(s);
                        stm._errIndx += yyleng;
                        return VAR_DECIMAL;
                    };
[a-zA-Z_]+[0-9]*     {
                        yylval->__c = (char*)malloc(yyleng + 1);
                        memcpy(yylval->__c, yytext, yyleng);
                        yylval->__c[yyleng] = '\0';
                        stm._errIndx += yyleng;
                        return VAR_STRING;
                    };
.
%%
/**
    1、flex模式只匹配输入字符或字符串一次
    2、flex执行当前输入的最长可能匹配的动作
    3. flex的规则有着隐藏优先级，即匹配长度相同时，将最上面的规则作为匹配结果
 */
// int yylex(yyscan_t* scan) {
//     return 1;
// }