#pragma once
#include <cstddef>
#include <vector>

struct GListNode;
class GArrayExpression {
public:
  using iterator = std::vector<GListNode*>::iterator;

  enum class ElementType {
    Undefined,
    Mixed,
    Number,
    Integer,
    String,
  };

  GArrayExpression();
  ~GArrayExpression();

  void addElement(GListNode* element);

  size_t size() const { return _elements.size(); }
  ElementType elementType() const { return _type; }

  iterator begin() { return _elements.begin(); }
  iterator end() { return _elements.end(); }

  GListNode* operator[](int idx) { return _elements[idx]; }

  void clear() { _elements.clear(); }
private:
  bool isBasicElement(GListNode*);

private:
  std::vector<GListNode*> _elements;
  ElementType _type;
};
