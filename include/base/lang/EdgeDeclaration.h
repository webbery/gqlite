#pragma once

struct GASTNode;
class GEdgeDeclaration {
public:
  GEdgeDeclaration(GASTNode* from, GASTNode* to, GASTNode* link);
  ~GEdgeDeclaration();

  GASTNode* link() const { return _link; }
  GASTNode* from() const { return _from; }
  GASTNode* to() const { return _to; }
private:
  GASTNode* _from;
  GASTNode* _to;
  GASTNode* _link;
};