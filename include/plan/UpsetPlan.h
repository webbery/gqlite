#pragma once
#include "Plan.h"
#include "Type/Binary.h"
#include "base/lang/lang.h"
#include "base/lang/AST.h"
#include "base/Variant.h"
#include "gutil.h"
#include "json.hpp"
#include "Graph/GRAD.h"
#include "operand/query/HNSW.h"

#define ATTRIBUTE_SET(item) \
  [&](int value) {\
    item[_key] = value;\
  },\
  [&](double value) {\
    item[_key] = value;\
  },\
  [&](std::string value) {\
    item[_key] = value;\
  },\
  [&](gql::GBinary value) {\
    item[_key] = nlohmann::json::binary(value.raw());\
  },\
  [&](gql::GDatetime value) {\
    item[_key] = { {"value", value.value()}, {OBJECT_TYPE_NAME, AttributeKind::Datetime} };\
  },\
  [&](gql::vector_double value) {\
    item[_key] = { {"value", value}, {OBJECT_TYPE_NAME, AttributeKind::Vector} };\
  }

#define ATTRIBUTE_PUSH(item) \
  [&](int value) {\
    item.push_back(value);\
  },\
  [&](double value) {\
    item.push_back(value);\
  },\
  [&](std::string value) {\
    item.emplace_back(value);\
  },\
  [&](gql::GBinary value) {\
    item.emplace_back(nlohmann::json::binary(value.raw()));\
  },\
  [&](gql::GDatetime value) {\
    item.push_back({ {"value", value.value()}, {OBJECT_TYPE_NAME, AttributeKind::Datetime} });\
  },\
  [&](gql::vector_double value) {\
    item[_key] = { {"value", value}, {OBJECT_TYPE_NAME, AttributeKind::Vector} };\
  }

struct GASTNode;
class GUpsetPlan: public GPlan {
public:
  GUpsetPlan(std::map<std::string, GVirtualNetwork*>& vn, GStorageEngine* store, GUpsetStmt* stmt);
  ~GUpsetPlan();

  virtual int prepare();
  virtual int execute(const std::function<ExecuteStatus(KeyType, const std::string& key, const std::string& value)>&);

private:
  /**
   * @brief JSONVisitor travels vertex/edge property and retrieve their infomation
   */
  struct JSONVisitor {
    nlohmann::json _jsonify;    /** read property as an json */
    std::string _key;           /** current read key */
    std::vector<attribute_t> _values; /** current read value in _key */
    //GUpsetPlan& _plan;
    JSONVisitor(GUpsetPlan& plan) {}
    
    VisitFlow apply(GASTNode* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GUpsetStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GQueryStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GGQLExpression* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GProperty* stmt, std::list<NodeType>& path) {
      add();
      _key = stmt->key();
      _values.clear();
      return VisitFlow::Children;
    }
    VisitFlow apply(GVertexDeclaration* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GCreateStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GDumpStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::SkipCurrent;
    }
    VisitFlow apply(GLiteral* stmt, std::list<NodeType>& path) {
      switch (stmt->kind()) {
      case AttributeKind::Binary:
      {
        gql::GBinary bin(stmt->raw().c_str());
        _values.emplace_back(bin);
      }
        break;
      case AttributeKind::Datetime:
      {
        gql::GDatetime date(atoll(stmt->raw().c_str()));
        _values.emplace_back(date);
      }
        break;
      case AttributeKind::Integer:
        _values.push_back(atoi(stmt->raw().c_str()));
        break;
      case AttributeKind::Number:
        _values.push_back(atof(stmt->raw().c_str()));
        break;
      case AttributeKind::String:
        _values.push_back(stmt->raw());
        break;
      default:
        break;
      }
      return VisitFlow::Children;
    }
    VisitFlow apply(GArrayExpression* stmt, std::list<NodeType>& path);
    VisitFlow apply(GEdgeDeclaration* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GDropStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GRemoveStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GObjectFunction* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }

    void add() {
      if (!_key.empty()) {
        if (_values.size() == 1) {
          _values[0].visit(ATTRIBUTE_SET(_jsonify));
        }
        else {
          for (auto& item : _values)
          {
            item.visit(ATTRIBUTE_PUSH(_jsonify[_key]));
          }
        }
      }
      else {
        if (_values.size() == 1) {
          _values[0].visit(ATTRIBUTE_SET(_jsonify));
        }
      }
    }
  };

  struct UpsetVisitor {
    GUpsetPlan& _plan;

    UpsetVisitor(GUpsetPlan& plan): _plan(plan) {}
    VisitFlow apply(GASTNode* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GUpsetStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GQueryStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GGQLExpression* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GProperty* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GDumpStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::SkipCurrent;
    }
    VisitFlow apply(GVertexDeclaration* stmt, std::list<NodeType>& path);
    VisitFlow apply(GCreateStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GLiteral* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GArrayExpression* stmt, std::list<NodeType>& path) {
      for (auto itr = stmt->begin(), end = stmt->end(); itr != end; ++itr) {
        VisitFlow vf = accept(*itr, *this, path);
        if (vf == VisitFlow::Children || vf == VisitFlow::SkipCurrent) continue;
        return vf;
      }
      return VisitFlow::Children;
    }
    VisitFlow apply(GEdgeDeclaration* stmt, std::list<NodeType>& path);
    VisitFlow apply(GDropStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GRemoveStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GObjectFunction* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    gkey_t getLiteral(GASTNode* node);
  };

  friend struct UpsetVisitor;

private:
  bool upsetVertex();
  bool upsetEdge();
  bool upsetIndex(const std::string& index, const std::string& value, uint64_t id);
  bool upsetIndex(const std::string& index, double value, uint64_t id);
  bool upsetIndex(const std::string& index, double value, const std::string& id);
  bool upsetIndex(const std::string& index, const std::string& value, const std::string& id);
  void addVectorIndex(const std::string& index, const std::string& id, const std::vector<double>& v);
  void addVectorIndex(const std::string& index, uint32_t id, const std::vector<double>& v);
  GVirtualNetwork* generateNetwork(const std::string& branch);

  template<typename T>
  bool upsetIndex(const nlohmann::json& item, const T& id) {
    for (auto& index : _indexes) {
      std::string k = index.substr(_class.size() + 1, index.size() - _class.size() - 1);
      if (item.count(k) == 0) continue;
      auto& value = item[k];
      if (value.is_object() && value.count(OBJECT_TYPE_NAME)) {
        switch ((AttributeKind)value[OBJECT_TYPE_NAME])
        {
        case AttributeKind::Vector:
        {
          if (!_hnsws.count(index)) {
            // group name + index name can fix identity name
            GVirtualNetwork* net = generateNetwork(index);
            _hnsws[index] = new GHNSW(net, _store, index.c_str(), (index + ":v").c_str());
          }
          addVectorIndex(index, id, value["value"]);
          _store->updateIndexType(index, IndexType::Vector);
        }
          break;
        default:
          break;
        }
      }
      else if (value.is_string()) {
        upsetIndex(index, (std::string)value, id);
      }
      else if (value.is_number_unsigned()) {
        uint64_t k = value;
        upsetIndex(index, k, id);
      }
      else if (value.is_number_integer()) {
        double k = value;
        upsetIndex(index, k, id);
      }
      else if (value.is_array()) {
        for (auto datum: value)
        {
          switch ((nlohmann::json::value_t)datum)
          {
          case nlohmann::json::value_t::string:
            upsetIndex(index, (std::string)datum, id);
            break;
          default:
            break;
          }
        }
      }
      else {
        printf("%s unknow type: %s\n", index.c_str(), value.type_name());
      }
    }
    return true;
  }
private:
  bool _vertex;       /**< true if upset target is vertex, else is edge */
  std::string _class;
  
  std::map<gkey_t, nlohmann::json> _vertexes;
  std::map<gql::edge_id, std::string> _edges;
  std::vector<std::string> _indexes;
  // 
  std::map<std::string, GHNSW*> _hnsws;
};