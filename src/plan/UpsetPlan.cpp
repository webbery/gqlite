#include <thread>
#include <future>
#include "plan/UpsetPlan.h"
#include "StorageEngine.h"
#include "VirtualNetwork.h"
#include "gutil.h"
#include <fmt/printf.h>
#include <fmt/color.h>

GUpsetPlan::GUpsetPlan(GVirtualNetwork* vn, GStorageEngine* store, GUpsetStmt* ast)
:GPlan(vn, store)
,_class(ast->name()) {
  UpsetVisitor visitor(*this);
  std::list<NodeType> ln;
  accept(ast->node(), visitor, ln);
}

GUpsetPlan::~GUpsetPlan()
{
  for (auto& item: _edges) {
    gql::release_edge_id(item.first);
  }
}

int GUpsetPlan::prepare() {
  // check graph is create or not.
  auto schema = _store->getSchema();
  if (schema.empty()) return ECode_Fail;
  return ECode_Success;
}

int GUpsetPlan::execute(const std::function<ExecuteStatus(KeyType, const std::string& key, const std::string& value)>& processor) {
  if (_store) {
    if (_vertex) return upsetVertex();
    else return upsetEdge();
  }
  return 0;
}

bool GUpsetPlan::upsetVertex()
{
  KeyType type = _store->getKeyType(_class);
  for (auto itr = _vertexes.begin(), end = _vertexes.end(); itr != end; ++itr) {
    const auto& key = itr->first;
    int ret = key.visit(
      [&](std::string k) {
        if (type == KeyType::Integer) {
          fmt::print(fmt::fg(fmt::color::red), "ERROR: upset fail!\nInput key type is string, but require integer\n");
          return ECode_Fail;
        }
        return _store->write(_class, k, itr->second.data(), itr->second.size());
      },
      [&](uint64_t k) {
        if (type == KeyType::Byte) {
          fmt::print(fmt::fg(fmt::color::red), "ERROR: upset fail!\nInput key type is integer, but require string\n");
          return ECode_Fail;
        }
        return _store->write(_class, k, itr->second.data(), itr->second.size());
      });
    if (ret != ECode_Success) return ret;
  }
  return ECode_Success;
}

bool GUpsetPlan::upsetEdge()
{
  for (auto itr = _edges.begin(), end = _edges.end(); itr != end; ++itr) {
    std::string sid = gql::to_string(itr->first);
    _store->write(_class, sid, itr->second.data(), itr->second.size());
  }
  return ECode_Success;
}

VisitFlow GUpsetPlan::UpsetVisitor::apply(GEdgeDeclaration* stmt, std::list<NodeType>& path)
{
  _plan._vertex = false;
  key_t from = getLiteral(stmt->from());
  key_t to = getLiteral(stmt->to());
  JSONVisitor jv(_plan);
  accept(stmt->link(), jv, path);
  jv.add();
  std::string edge = jv._jsonify.dump();
  if (edge == "->" || edge == "<-") {}
  else {
    gql::edge_id eid = gql::make_edge_id(false, from, to);
    _plan._edges[eid] = edge;
  }
  return VisitFlow::Return;
}

VisitFlow GUpsetPlan::UpsetVisitor::apply(GVertexDeclaration* stmt, std::list<NodeType>& path)
{
  _plan._vertex = true;
  JSONVisitor jv(_plan);
  accept(stmt->vertex(), jv, path);
  jv.add();
  _plan._vertexes[getLiteral(stmt->key())] = jv._jsonify.dump();
  return VisitFlow::Return;
}

key_t GUpsetPlan::UpsetVisitor::getLiteral(GASTNode* node)
{
  GLiteral* literal = (GLiteral*)(node->_value);
  key_t k;
  switch (literal->kind()) {
  case AttributeKind::Number:
    k = (uint64_t)atoll(literal->raw().c_str());
    break;
  default:
    k = literal->raw();
    break;
  }
  return k;
}
