#include "VirtualEngine.h"
#include <chrono>
#include <future>
#include "Platform.h"
#include "Memory.h"
#include "ISymbol.h"
#include "Statement.h"
#ifdef _WIN32
#include <io.h>
#define isatty(x) _isatty(x)
#else
#include <unistd.h>
#endif

// #if !YYDEBUG
//   static int yydebug;
// #endif
union YYSTYPE;
struct YYLTYPE;
#include "base/list.h"
int yylex(YYSTYPE* yylval_param, YYLTYPE* yylloc_param, void* yyscanner, GStatement& stm);
#include "./gql.cpp"
#include "./token.cpp"

#define VALUE_START  RANGE_BEGIN
#define VALUE_END    EOL

GVirtualEngine::GVirtualEngine()
:_top(-1)
{
  
  //_tInterpret = std::thread(&GVirtualEngine::execute, this);
}

GVirtualEngine::~GVirtualEngine()
{
  //_tInterpret.join();
}

void GVirtualEngine::test(GStatement& stm)
{
  // https://stackoverflow.com/questions/62820971/passing-an-argument-to-yylex
  //yydebug = 1;
  yyscan_t scanner;
  yylex_init(&scanner);
  //yyset_debug(1, scanner);
  // struct yyguts_t * yyg = (struct yyguts_t*)scanner;
  // yy_flex_debug = 1;
  yy_scan_string(stm.gql().c_str(), scanner);
  int tok = 0;
  int ret = yyparse(scanner, stm);
  yylex_destroy(scanner);
}

void GVirtualEngine::addNoSQL(const NoSQLRoutine& nosql)
{
  //this->_nosqls[this->_top] = instruction;
  this->_top += 1;
}

void GVirtualEngine::generateInternalRepresent(AST* pAst)
{
  //pAst->DFS([this](ISymbol* symbol) {

  //});
}

void GVirtualEngine::awaitExecute(NoSQLRoutine& instruction)
{
  std::unique_lock<std::mutex> ul(_m);
  this->interpret(instruction);
  _cv.notify_one();
}

void GVirtualEngine::execute()
{
  extern std::atomic<bool> _gqlite_g_close_flag_;
}

void GVirtualEngine::interpret(NoSQLRoutine& i)
{
  //switch (i._ops)
  //{
  //case GOp::OP_GraphDB:
  //  i._v1 = _pStorageEngine->create(i._v3.s);
  //  break;
  //default:
  //  break;
  //}
}
