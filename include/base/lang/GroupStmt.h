#pragma once
#include <string>

struct GListNode;
class GGroupStmt {
public:
  enum GroupType {
    Vertex,
    Edge,
  };

  GGroupStmt(const std::string& name, GListNode* props = nullptr, GListNode* indexes = nullptr);
  virtual ~GGroupStmt();

  std::string name() const { return _name; }
  GListNode* properties() { return _props; }
  GListNode* indexes() { return _indexes; }

  GroupType type() { return _type; }
protected:
  GroupType _type;
  std::string _name;
  GListNode* _props;
  GListNode* _indexes;
};

class GVertexGroupStmt : public GGroupStmt {
public:
  GVertexGroupStmt(const std::string& name, GListNode* props = nullptr, GListNode* indexes = nullptr);

};

class GEdgeGroupStmt : public GGroupStmt {
public:
  GEdgeGroupStmt(const std::string& name, GListNode* props, const std::string& from, const std::string& to);

  const std::string& from() const { return _from; }
  const std::string& to() const { return _to; }

private:
  std::string _from;
  std::string _to;
};