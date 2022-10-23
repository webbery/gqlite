#pragma once
#include <vector>

struct GASTNode;
class GArrayExpression {
public:
  using iterator = std::vector<GASTNode*>::iterator;

  enum class ElementType {
    Undefined,
    Mixed,
    Number,
    Integer,
    String,
  };

  GArrayExpression();
  ~GArrayExpression();

  void addElement(GASTNode* element);

  size_t size() const { return _elements.size(); }
  ElementType elementType() const { return _type; }

  iterator begin() { return _elements.begin(); }
  iterator end() { return _elements.end(); }

  GASTNode* operator[](int idx) { return _elements[idx]; }
private:
  bool isBasicElement(GASTNode*);

private:
  std::vector<GASTNode*> _elements;
  ElementType _type;
};

GASTNode* InitList(struct GASTNode* item);