#pragma once
#include <string>

struct GASTNode;
class GRemoveStmt {
public:
  enum RemoveType{
    Vertex,
    Edge
  };
  GRemoveStmt(const std::string& name, GASTNode* array);
  virtual ~GRemoveStmt();

  std::string name() const { return _name; }
  GASTNode* node() const { return _array; }
  RemoveType type() const { return _type; }
protected:
  RemoveType _type;
  std::string _name;
  GASTNode* _array;
};

class GVertexRemoveStmt : public GRemoveStmt {
public:
  GVertexRemoveStmt(const std::string& name, GASTNode* array);
};

class GEdgeRemoveStmt : public GRemoveStmt {
public:
  GEdgeRemoveStmt(const std::string& name, GASTNode* array);
};