#pragma once

struct GListNode;
class GVertexDeclaration {
public:
  GVertexDeclaration(GListNode* key, GListNode* nodes);
  ~GVertexDeclaration();

  GListNode* key() const { return _key; }
  GListNode* vertex() const { return _head; }
private:
  GListNode* _key;
  GListNode* _head;
};