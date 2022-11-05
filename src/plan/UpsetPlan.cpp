#include <thread>
#include <future>
#include "plan/UpsetPlan.h"
#include "StorageEngine.h"
#include "VirtualNetwork.h"
#include "gutil.h"
#include <fmt/printf.h>
#include <fmt/color.h>

GUpsetPlan::GUpsetPlan(std::map<std::string, GVirtualNetwork*>& vn, GStorageEngine* store, GUpsetStmt* ast)
:GPlan(vn, store)
,_class(ast->name())
{
  UpsetVisitor visitor(*this);
  std::list<NodeType> ln;
  accept(ast->node(), visitor, ln);
  _indexes = _store->getIndexes();
}

GUpsetPlan::~GUpsetPlan()
{
  for (auto& item: _edges) {
    gql::release_edge_id(item.first);
  }
  for(auto hnsw: _hnsws) {
    delete hnsw.second;
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
        if (_store->write(_class, k, itr->second) == ECode_Success){
          //printf("write: %s\n", itr->second.dump().c_str());
          if (!_indexes.empty()) {
            upsetIndex(itr->second, k);
            return ECode_Success;
          }
        }
        return ECode_Fail;
      },
      [&](uint64_t k) {
        if (type == KeyType::Byte) {
          fmt::print(fmt::fg(fmt::color::red), "ERROR: upset fail!\nInput key type is integer, but require string\n");
          return ECode_Fail;
        }
        if (_store->write(_class, k, itr->second) == ECode_Success){
          //printf("write: %s\n", itr->second.dump().c_str());
          if (!_indexes.empty()) {
            upsetIndex(itr->second, k);
            return ECode_Success;
          }
        }
        return ECode_Fail;
      });
    if (ret != ECode_Success) return ret;
  }
  return ECode_Success;
}

bool GUpsetPlan::upsetEdge()
{
  _store->tryInitKeyType(_class, KeyType::Edge);
  for (auto itr = _edges.begin(), end = _edges.end(); itr != end; ++itr) {
    std::string sid = gql::to_string(itr->first);
    _store->write(_class, sid, itr->second.data(), itr->second.size());
  }
  return ECode_Success;
}

void GUpsetPlan::addVectorIndex(const std::string& index, const std::string& id, const std::vector<double>& v)
{
  uint64_t uid = gql::hash64(id);
  _hnsws[index]->add((node_t)uid, v);
}

void GUpsetPlan::addVectorIndex(const std::string& index, uint32_t id, const std::vector<double>& v)
{
  _hnsws[index]->add((node_t)id, v);
}

GVirtualNetwork* GUpsetPlan::generateNetwork(const std::string& branch)
{
  if (!_networks[branch]) {
    _networks[branch] = new GVirtualNetwork(1024);
  }
  return _networks[branch];
}

bool GUpsetPlan::upsetIndex(const std::string& index, const std::string& value, const std::string& id)
{
  std::string data;
  _store->read(index, value, data);
  std::vector<std::string> v = gql::split(data, '\0');
  addUniqueDataAndSort(v, id);
  data.clear();
  for (auto& datum : v) {
    data += datum + '\0';
  }
  data.pop_back();
  _store->write(index, value, data.data(), data.size() * sizeof(char));
  _store->updateIndexType(index, IndexType::Word);
  return true;
}

bool GUpsetPlan::upsetIndex(const std::string& index, double value, uint64_t id)
{
  std::string bin((char*)&value, sizeof(double));
  std::string data;
  _store->read(index, bin, data);
  std::vector<uint64_t> v((uint64_t*)data.data(), (uint64_t*)data.data() + data.size() / sizeof(uint64_t));
  addUniqueDataAndSort(v, id);
  _store->write(index, bin, v.data(), v.size());
  _store->updateIndexType(index, IndexType::Number);
  return true;
}

bool GUpsetPlan::upsetIndex(const std::string& index, const std::string& value, uint64_t id)
{
  std::string data;
  _store->read(index, value, data);
  std::vector<uint64_t> v((uint64_t*)data.data(), (uint64_t*)data.data() + data.size() / sizeof(uint64_t));
  addUniqueDataAndSort(v, id);
  _store->write(index, value, v.data(), v.size());
  _store->updateIndexType(index, IndexType::Word);
  return true;
}

bool GUpsetPlan::upsetIndex(const std::string& index, double value, const std::string& id)
{
  std::string bin((char*)&value, sizeof(double));
  std::string data;
  _store->read(index, bin, data);
  std::vector<std::string> v = gql::split(data, '\0');
  addUniqueDataAndSort(v, id);
  data.clear();
  for (auto& datum : v) {
    data += datum + '\0';
  }
  data.pop_back();
  _store->write(index, bin, data.data(), data.size() * sizeof(char));
  _store->updateIndexType(index, IndexType::Number);
  return true;
}

VisitFlow GUpsetPlan::UpsetVisitor::apply(GEdgeDeclaration* stmt, std::list<NodeType>& path)
{
  _plan._vertex = false;
  gkey_t from = getLiteral(stmt->from());
  gkey_t to = getLiteral(stmt->to());
  JSONVisitor jv(_plan);
  accept(stmt->value(), jv, path);
  jv.add();
  std::string edge = jv._jsonify.dump();
  if (stmt->direction() == "->") {
    gql::edge_id eid = gql::make_edge_id(true, from, to);
    _plan._edges[eid] = edge;
  }
  else if (stmt->direction() == "<-") {
    gql::edge_id eid = gql::make_edge_id(true, to, from);
    _plan._edges[eid] = edge;
  }
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
  _plan._vertexes[getLiteral(stmt->key())] = jv._jsonify;
  return VisitFlow::Children;
}

gkey_t GUpsetPlan::UpsetVisitor::getLiteral(GASTNode* node)
{
  GLiteral* literal = (GLiteral*)(node->_value);
  gkey_t k;
  switch (literal->kind()) {
  case AttributeKind::Number:
    k = (uint64_t)atoll(literal->raw().c_str());
    break;
  case AttributeKind::Datetime:
    break;
  default:
    k = literal->raw();
    break;
  }
  return k;
}

VisitFlow GUpsetPlan::JSONVisitor::apply(GArrayExpression* stmt, std::list<NodeType>& path)
{
  switch (stmt->elementType())
  {
  case GArrayExpression::ElementType::Number:
  case GArrayExpression::ElementType::Integer:
  {
    auto itr = stmt->begin();
    gql::vector_double vec;
    while (itr != stmt->end()) {
      GLiteral* ptr = (GLiteral*)(*itr)->_value;
      switch (ptr->kind()) {
      case AttributeKind::Integer:
        vec.push_back(atoi(ptr->raw().c_str()));
        break;
      case AttributeKind::Number:
        vec.push_back(atof(ptr->raw().c_str()));
        break;
      default:
        break;
      }
      ++itr;
    }
    _values.push_back(vec);
  }
  return VisitFlow::SkipCurrent;
  default:
    return VisitFlow::Children;
  }
}
