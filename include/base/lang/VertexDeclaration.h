#pragma once

struct GASTNode;
class GVertexDeclaration {
public:
  GVertexDeclaration(GASTNode* node);

  GASTNode* vertex() const { return _head; }
private:
  GASTNode* _head;
};