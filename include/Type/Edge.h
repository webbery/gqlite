#pragma once
#include "json.hpp"

enum class GraphEdgeDirection {
  Bidrection,
  To,
};

class edge_id {
public:
  edge_id(const std::string& from, const std::string& to, GraphEdgeDirection direction = GraphEdgeDirection::Bidrection);
  edge_id(const std::string& eid);

  bool operator == (const edge_id& other);
  bool operator == (const edge_id& other) const;
  bool operator != (const edge_id& other);
  bool operator != (const edge_id& other) const;
  bool operator < (const edge_id& other);
  bool operator < (const edge_id& other) const;

  operator std::string();
  operator std::string()const;

  std::string from()const;
  std::string to()const;
  bool isDirection()const;

private:
  std::string str()const;
  bool equal(const edge_id& other) const;
  bool lt(const edge_id& other)const;
private:
  std::string _from;
  std::string _to;
  GraphEdgeDirection _direction;
};

//bool operator<(const edge_id& e1, const edge_id& e2);
//bool operator!=(const edge_id& e1, const edge_id& e2);

class GLiteralEdge {};

class GAttributeEdge {};

class GEdge {
public:
  GEdge(const std::string& from, const std::string& to, GraphEdgeDirection direction);
  virtual ~GEdge() {}

  std::string id();
  std::string from()const;
  std::string to()const;
  std::string to(const std::string& from)const;
  void set(const nlohmann::json& prop) {
    _json = prop;
  }
  nlohmann::json prop(const std::string& name) {
    return _json[name];
  }
  std::string toString();
protected:
  edge_id _id;
  nlohmann::json _json;
};
