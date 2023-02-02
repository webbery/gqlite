#pragma once
#include <string>

struct GListNode;
class GRemoveStmt {
public:
  enum RemoveType{
    Vertex,
    Edge
  };
  GRemoveStmt(const std::string& name, GListNode* array);
  virtual ~GRemoveStmt();

  std::string name() const { return _name; }
  GListNode* node() const { return _array; }
  RemoveType type() const { return _type; }
protected:
  RemoveType _type;
  std::string _name;
  GListNode* _array;
};

class GVertexRemoveStmt : public GRemoveStmt {
public:
  GVertexRemoveStmt(const std::string& name, GListNode* array);
};

class GEdgeRemoveStmt : public GRemoveStmt {
public:
  GEdgeRemoveStmt(const std::string& name, GListNode* array);
};