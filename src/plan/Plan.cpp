#include "plan/Plan.h"
#include "base/gvm/Chunk.h"
#include "base/gvm/Compiler.h"

void init_result_nodes(gqlite_result& result)
{

}

GPlan::GPlan(std::map<std::string, GVirtualNetwork*>& networks, GStorageEngine* store)
: _left(nullptr), _right(nullptr)
, _chunk(nullptr)
, _compiler(nullptr)
, _store(store)
, _networks(networks) {}

GPlan::~GPlan() {
  if (_left) delete _left;
  if (_right) delete _right;
  if (_chunk) delete _chunk;
  if (_compiler) delete _compiler;
}