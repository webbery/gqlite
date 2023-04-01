#include "plan/query/QueryPlan.h"
#include "Context.h"
#include "plan/query/ScanPlan.h"
#include "StorageEngine.h"
#include "gutil.h"
#include "base/gvm/GVM.h"

namespace {
  void init_vertex(gqlite_vertex* vertex, uint8_t type, std::string& sID) {
    if (type == 0) {
      vertex->type = gqlite_id_type::integer;
      vertex->uid = *(uint64_t*)sID.data();
    }
    else {
      vertex->type = gqlite_id_type::bytes;
      sID += "\0";
      vertex->cid = (char*)sID.data();
    }
    vertex->properties = nullptr;
  }
}

GQueryPlan::GQueryPlan(GContext* context, GQueryStmt* stmt, gqlite_callback cb, void* cbHandle)
  :GPlan(context->_graph, context->_storage, context->_schedule)
  , _cb(cb), _handle(cbHandle)
{
  _scan = new GScanPlan(context, stmt);
}

GQueryPlan::~GQueryPlan()
{
  delete _scan;
}

int GQueryPlan::prepare()
{
  return _scan->prepare();
}

void GQueryPlan::addCompiler(Compiler* c) {
  _scan->addCompiler(c);
}


int GQueryPlan::execute(GVM* gvm, const std::function<ExecuteStatus(KeyType, const std::string& key, nlohmann::json& value, int status)>& processor)
{
  if (_cb) {
    _scan->execute(gvm, [this](KeyType type, const std::string& key, nlohmann::json& value, int status) {
      gqlite_result result;
      result.count = 1;
      result.type = gqlite_result_type_node;
      result.errcode = status;
      result.nodes = new gqlite_node;
      result.nodes->_next = nullptr;
      if (type != KeyType::Edge) {
        convert_vertex(type, key, value, result);
      }
      else {
        convert_edge(key, value, result);
      }
      delete result.nodes;
      return ExecuteStatus::Continue;
    });
    
  }
  return 0;
}

void GQueryPlan::beautify(nlohmann::json& input)
{
  if (input.empty() || input.is_null()) return;
  if (input.is_object()) {
    if (input.count(OBJECT_TYPE_NAME)) {
      switch (AttributeKind(input[OBJECT_TYPE_NAME])) {
      case AttributeKind::Datetime:
        input = std::string("0d") + std::to_string((time_t)input["value"]);
        return;
      case AttributeKind::Vector:
        input = input["value"];
        return;
      default:
        break;
      }
    }
    else if (input.count("bytes") && input.count("subtype")) {
      // binary
      std::vector<uint8_t> bin = input["bytes"];
      input = std::string("0b") + gql::base64_encode(bin);
      return;
    }
    for (auto itr = input.begin(); itr != input.end(); ++itr)
    {
      beautify(itr.value());
    }
  }
  else if (input.is_array()) {
    for (auto itr = input.begin(); itr != input.end(); ++itr)
    {
      beautify(itr.value());
    }
  }
}

void GQueryPlan::convert_vertex(KeyType type, const std::string& key, nlohmann::json& jsn, gqlite_result& result)
{
  if (result.errcode != ECode_Success) {
    gqlite_result err;
    err.errcode = result.errcode;
    err.type = gqlite_result_type::gqlite_result_type_info;
    err.count = 1;
    const char* p[1] = { key.c_str() };
    err.infos = (char**)p;
    _cb(&err, _handle);
    return;
  }

  beautify(jsn);
  std::string value = jsn.dump();

  result.nodes->_type = gqlite_node_type::gqlite_node_type_vertex;
  result.nodes->_vertex = new gqlite_vertex;
  if (type == KeyType::Integer) {
    result.nodes->_vertex->type = gqlite_id_type::integer;
    result.nodes->_vertex->uid = *(uint64_t*)key.data();
  }
  else {
    result.nodes->_vertex->type = gqlite_id_type::bytes;
    result.nodes->_vertex->cid = (char*)key.data();
  }
  
  size_t len = value.size();
  result.nodes->_vertex->properties = new char[len + 1];
  memcpy(result.nodes->_vertex->properties, value.data(), sizeof(char) * len + 1);
  _cb(&result, _handle);
  delete[] result.nodes->_vertex->properties;
  delete result.nodes->_vertex;
}

void GQueryPlan::convert_edge(const std::string& key, nlohmann::json& jsn, gqlite_result& result)
{
  result.nodes->_type = gqlite_node_type::gqlite_node_type_edge;
  result.nodes->_edge = new gqlite_edge;

  auto id = gql::to_edge_id(key);
  std::string idFrom(id._value, id._from_len);
  result.nodes->_edge->from = new gqlite_vertex;
  init_vertex(result.nodes->_edge->from, id._from_type, idFrom);
  result.nodes->_edge->to = new gqlite_vertex;
  std::string idTo(id._value + id._from_len, id._len - id._from_len);
  init_vertex(result.nodes->_edge->to, id._to_type, idTo);
  result.nodes->_edge->direction = id._direction;

  std::string value = jsn.dump();
  size_t len = value.size();
  if (value != "null") {
    result.nodes->_edge->properties = new char[len + 1];
    memcpy(result.nodes->_edge->properties, value.data(), sizeof(char) * len + 1);
    result.nodes->_edge->len = len;
  }
  else {
    result.nodes->_edge->properties = nullptr;
    result.nodes->_edge->len = 0;
  }

  _cb(&result, _handle);

  if (result.nodes->_edge->properties) delete[] result.nodes->_edge->properties;
  delete result.nodes->_edge->to;
  delete result.nodes->_edge->from;
  gql::release_edge_id(id);
  delete result.nodes->_edge;
}
