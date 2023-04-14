#include <thread>
#include <future>
#include "Context.h"
#include "plan/mutate/UpsetPlan.h"
#include "plan/query/ScanPlan.h"
#include "StorageEngine.h"
#include "VirtualNetwork.h"
#include "gutil.h"
#include <fmt/printf.h>
#include <fmt/color.h>
#include "base/system/Coroutine.h"

GUpsetPlan::GUpsetPlan(GContext* context, GUpsetStmt* ast)
:GPlan(context->_graph, context->_storage, context->_schedule)
,_class(ast->name())
,_scan(nullptr)
{
  GListNode* condition = ast->conditions();
  if (condition) {
    _scan = new GScanPlan(context, condition, _class);
  }
  UpsetVisitor visitor(*this);
  std::list<NodeType> ln;
  accept(ast->node(), &visitor, ln);
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
  if (_scan) delete _scan;
}

int GUpsetPlan::prepare() {
  // check graph is create or not.
  auto schema = _store->getSchema();
  if (schema.empty()) return ECode_Fail;

  if (_scan) return _scan->prepare();
  return ECode_Success;
}

int GUpsetPlan::execute(GVM* gvm, const std::function<ExecuteStatus(KeyType, const std::string& key, nlohmann::json& value, int status)>& processor) {
  if (_store) {
    if (!_scan) {
      if (_vertex) return upsetVertex();
      else return upsetEdge();
    }
    else {
      _scan->execute(gvm, [&](KeyType type, const std::string& key, nlohmann::json& value, int status) {
        if (status != ECode_Success) return ExecuteStatus::Stop;

        for (auto& item : _props.items()) {
          std::string k = item.key();
          value[k] = item.value();
        }

        if (type == KeyType::Integer) {
          uint64_t upsetKey = *(uint64_t*)key.data();
          if (_store->write(_class, upsetKey, value) == ECode_Success) {
            for (auto& item : _props.items()) {
              upsetIndex(item, upsetKey);
            }
            return ExecuteStatus::Stop;
          }
        }
        else if (type == KeyType::Byte) {
          std::string upsetKey(key.data(), key.size());
          if (_store->write(_class, upsetKey, value) == ECode_Success) {
            for (auto& item : _props.items()) {
              upsetIndex(item, upsetKey);
            }
            return ExecuteStatus::Stop;
          }
        }
        return ExecuteStatus::Continue;
        });
    }
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
        if (_store->write(_class, k, itr->second) != ECode_Success)
          return ECode_Fail;
        
        if (!_indexes.empty()) {
          upsetIndex(itr->second, k);
        }

        auto relations = _store->getRelations(_class);
        if (relations.size()) {}
        return ECode_Success;
      },
      [&](uint64_t k) {
        if (type == KeyType::Byte) {
          fmt::print(fmt::fg(fmt::color::red), "ERROR: upset fail!\nInput key type is integer, but require string\n");
          return ECode_Fail;
        }
        if (_store->write(_class, k, itr->second) == ECode_Success){
          // printf("write: %s\n", itr->second.dump().c_str());
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
    _store->write(_class, sid, (void*)itr->second.data(), itr->second.size());
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
  if (!_network.count(branch)) {
    _network[branch] = new GVirtualNetwork(_schedule, 1024);
  }
  return _network[branch];
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
  _store->write(index, value, (void*)data.data(), data.size() * sizeof(char));
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
  _store->write(index, bin, (void*)data.data(), data.size() * sizeof(char));
  _store->updateIndexType(index, IndexType::Number);
  return true;
}

VisitFlow GUpsetPlan::UpsetVisitor::apply(GEdgeDeclaration* stmt, std::list<NodeType>& path)
{
  _plan._vertex = false;
  gkey_t from = getLiteral(stmt->from());
  gkey_t to = getLiteral(stmt->to());
  JSONVisitor jv(_plan._vertex);
  accept(stmt->value(), &jv, path);
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
  JSONVisitor jv(_plan._vertex);
  accept(stmt->vertex(), &jv, path);
  jv.add();
  _plan._vertexes[getLiteral(stmt->key())] = jv._jsonify;
  return VisitFlow::Children;
}

VisitFlow GUpsetPlan::UpsetVisitor::apply(GProperty* stmt, std::list<NodeType>& path)
{
  if (_plan._scan) {
    JSONVisitor jv(_plan._vertex);
    accept(stmt->value(), &jv, path);
    jv.add();
    for (auto& item : jv._jsonify.items()) {
      _plan._props[stmt->key()] = item.value();
    }
  }
  return VisitFlow::SkipCurrent;
}

gkey_t GUpsetPlan::UpsetVisitor::getLiteral(GListNode* node)
{
  GLiteral* literal = (GLiteral*)(node->_value);
  gkey_t k;
  switch (literal->kind()) {
  case AttributeKind::Integer:
  case AttributeKind::Number:
    k = (uint64_t)atoll(literal->raw().c_str());
    break;
  case AttributeKind::Datetime:
  case AttributeKind::Vector:
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
