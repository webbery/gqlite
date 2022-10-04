#include "GQliteImpl.h"
#include "base/system/Platform.h"
#include "VirtualEngine.h"
#include "Error.h"
#include "Memory.h"
#include "StorageEngine.h"
#ifdef _WIN32
#include <io.h>
#define isatty(x) _isatty(x)
#else
#include <unistd.h>
#endif

union YYSTYPE;
struct YYLTYPE;
int yylex(YYSTYPE* yylval_param, YYLTYPE* yylloc_param, void* yyscanner, GVirtualEngine& stm);
#include "./gql.cpp"
#include "./token.cpp"

#define VALUE_START  RANGE_BEGIN
#define VALUE_END    EOL

GQLiteImpl::GQLiteImpl(GVirtualEngine* pEng)
  : _ve(pEng)
{}

GQLiteImpl::~GQLiteImpl()
{
  this->close();
  if (_ve) {
    delete _ve;
  }
}

int GQLiteImpl::open(const char* filename, gqlite_open_mode mode)
{
  return create(filename, mode);
}

int GQLiteImpl::close()
{
  if (_ve->_storage) {
    delete _ve->_storage;
    return true;
  }
  return true;
}

int GQLiteImpl::create(const char* filename, gqlite_open_mode mode)
{
  if (filename) {
    StoreOption opt;
    opt.compress = 1;
    if (_ve->_storage == nullptr) {
      _ve->_storage = new GStorageEngine();
    }
    return _ve->_storage->open(filename, opt);
  }
  else {
    if (_ve->_storage) {
      delete _ve->_storage;
    }
    _ve->_storage = new GStorageEngine();
  }
  return ECode_Success;
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