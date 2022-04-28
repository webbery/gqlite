#include "GQliteImpl.h"
#include "ParserEngine.h"
#include "Platform.h"
#include "VirtualEngine.h"
#include "Error.h"
#include "Memory.h"
#include "StorageEngine.h"
#include "SubGraph.h"
#include "ISymbol.h"
#ifdef _WIN32
#include <io.h>
#define isatty(x) _isatty(x)
#else
#include <unistd.h>
#endif

union YYSTYPE;
struct YYLTYPE;
#include "base/list.h"
int yylex(YYSTYPE* yylval_param, YYLTYPE* yylloc_param, void* yyscanner, GVirtualEngine& stm);
#include "./gql.cpp"
#include "./token.cpp"

#define VALUE_START  RANGE_BEGIN
#define VALUE_END    EOL

GQLiteImpl::GQLiteImpl()
  : _statement(new GVirtualEngine)
{}

GQLiteImpl::~GQLiteImpl()
{
  this->close();
  if (_statement) {
    delete _statement;
  }
}

int GQLiteImpl::open(const char* filename, gqlite_open_mode mode)
{
  return create(filename, mode);
}

int GQLiteImpl::close()
{
  if (_statement->_storage) {
    return _statement->_storage->closeGraph(nullptr);
  } else {
    delete _statement->_graph;
    _statement->_graph = nullptr;
    return true;
  }
}

void GQLiteImpl::set(GVirtualEngine* pStatement)
{
  if (_statement) {
    delete _statement;
  }
  _statement = pStatement;
}

int GQLiteImpl::create(const char* filename, gqlite_open_mode mode)
{
  if (filename) {
    return _statement->_storage->create(filename);
  } else {
    if (_statement->_graph) delete _statement->_graph;
    _statement->_graph = new GSubGraph();
    return ECode_Success;
  }
}

void GQLiteImpl::exec(GVirtualEngine& stm)
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