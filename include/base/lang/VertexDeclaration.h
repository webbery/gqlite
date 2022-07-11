#pragma once

struct GASTNode;
class GVertexDeclaration {
public:
  GVertexDeclaration(GASTNode* key, GASTNode* nodes);
  ~GVertexDeclaration();

  GASTNode* key() const { return _key; }
  GASTNode* vertex() const { return _head; }
private:
  GASTNode* _key;
  GASTNode* _head;
};