#include "VirtualEngine.h"
#include "gqlite.h"
#include "StorageEngine.h"

uint32_t GVirtualEngine::_indx = 0;

uint32_t GVirtualEngine::GenerateIndex()
{
  return ++GVirtualEngine::_indx;
}

GVirtualEngine::GVirtualEngine()
: _errIndx(0)
, _errorCode(ECode_GQL_Parse_Fail)
, _cmdtype(GQL_Command_Size)
{
}

GVirtualEngine::~GVirtualEngine() {
  if (_graph) {
    delete _graph;
    _graph = nullptr;
  }
}

int GVirtualEngine::execAST(gast* ast) {
  return ECode_Success;
}