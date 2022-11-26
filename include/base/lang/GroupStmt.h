#pragma once
#include <string>

struct GASTNode;
class GGroupStmt {
public:
  enum GroupType {
    Vertex,
    Edge,
  };

  GGroupStmt(const std::string& name, GASTNode* props = nullptr, GASTNode* indexes = nullptr);
  virtual ~GGroupStmt();

  std::string name() const { return _name; }
  GASTNode* properties() { return _props; }
  GASTNode* indexes() { return _indexes; }

  GroupType type() { return _type; }
protected:
  GroupType _type;
  std::string _name;
  GASTNode* _props;
  GASTNode* _indexes;
};

class GVertexGroupStmt : public GGroupStmt {
public:
  GVertexGroupStmt(const std::string& name, GASTNode* props = nullptr, GASTNode* indexes = nullptr);

};

class GEdgeGroupStmt : public GGroupStmt {
public:
  GEdgeGroupStmt(const std::string& name, GASTNode* props, const std::string& from, const std::string& to);

  const std::string& from() const { return _from; }
  const std::string& to() const { return _to; }

private:
  std::string _from;
  std::string _to;
};