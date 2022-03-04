#include "Statement.h"
#include "gqlite.h"

uint32_t GStatement::_indx = 0;

uint32_t GStatement::GenerateIndex()
{
  return ++GStatement::_indx;
}

GStatement::GStatement(const char* gql, gqlite_callback cb)
: _result_callback(cb)
, _errIndx(0)
, _errorCode(ECode_GQL_Parse_Fail)
, _cmdtype(GQL_Command_Size)
, _gql(gql) {}
