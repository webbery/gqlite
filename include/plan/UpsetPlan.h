#pragma once
#include "Plan.h"
#include "base/lang/lang.h"
#include "base/lang/AST.h"
#include "base/Variant.h"
#include "json.hpp"

struct GASTNode;
class GUpsetPlan: public GPlan {
public:
  GUpsetPlan(GVirtualNetwork* vn, GStorageEngine* store, GUpsetStmt* stmt);

  virtual int prepare();
  virtual int execute(gqlite_callback);

private:
  using key_t = Variant<std::string, uint64_t>;

  /**
   * @brief JSONVisitor travels vertex/edge property and retrieve their infomation
   */
  struct JSONVisitor {
    nlohmann::json _jsonify;    /** read property as an json */
    std::string _key;           /** current read key */
    using var_t = Variant<std::string, double, int>;
    std::vector<var_t> _values; /** current read value in _key */
    GUpsetPlan& _plan;
    JSONVisitor(GUpsetPlan& plan): _plan(plan) {}
    
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
    VisitFlow apply(GLiteral* stmt, std::list<NodeType>& path) {
      switch (stmt->kind()) {
      case AttributeKind::Binary:
        // TODO:
        break;
      case AttributeKind::Datetime:
        // TODO:
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
    VisitFlow apply(GArrayExpression* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GEdgeDeclaration* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GDropStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }

    void add() {
      if (!_key.empty()) {
        if (_values.size() == 1) {
          _values[0].visit(
            [&](int value) {
              _jsonify[_key] = value;
            },
            [&](double value) {
              _jsonify[_key] = value;
            },
            [&](std::string value) {
              _jsonify[_key] = value;
            });
        }
        else {
          for (auto& item : _values)
          {
            item.visit(
              [&](int value) {
                _jsonify[_key].push_back(value);
              },
              [&](double value) {
                _jsonify[_key].push_back(value);
              },
              [&](std::string value) {
                _jsonify[_key].emplace_back(value);
              });
          }
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
    VisitFlow apply(GVertexDeclaration* stmt, std::list<NodeType>& path) {
      _plan._vertex = true;
      GLiteral* literal = (GLiteral*)(stmt->key()->_value);
      key_t k;
      switch (literal->kind()) {
      case AttributeKind::Number:
        k = atoi(literal->raw().c_str());
        // printf("upset key: %d\n", k.Get<int>());
        break;
      default:
        k = literal->raw();
        // printf("upset key: %s\n", k.Get<std::string>().c_str());
        break;
      }
      JSONVisitor jv(_plan);
      accept(stmt->vertex(), jv, path);
      jv.add();
      _plan._vertexes[k] = jv._jsonify.dump();
      return VisitFlow::Children;
    }
    VisitFlow apply(GCreateStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
    VisitFlow apply(GLiteral* stmt, std::list<NodeType>& path) {
      return VisitFlow::Children;
    }
    VisitFlow apply(GArrayExpression* stmt, std::list<NodeType>& path) {
      for (auto itr = stmt->begin(), end = stmt->end(); itr != end; ++itr) {
        accept(*itr, *this, path);
      }
      return VisitFlow::Children;
    }
    VisitFlow apply(GEdgeDeclaration* stmt, std::list<NodeType>& path) {
      _plan._vertex = false;
      return VisitFlow::Children;
    }
    VisitFlow apply(GDropStmt* stmt, std::list<NodeType>& path) {
      return VisitFlow::Return;
    }
  };

  friend struct UpsetVisitor;
private:
  bool _vertex;       /**< true if upset target is vertex, else is edge */
  std::string _class;
  std::map<key_t, std::string> _vertexes;
};