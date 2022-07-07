#pragma once
#include <vector>

struct GASTNode;
class GArrayExpression {
public:
  using iterator = std::vector<GASTNode*>::iterator;

  GArrayExpression();

  void addElement(GASTNode* element);

  size_t size() { return _elements.size(); }

  iterator begin() { return _elements.begin(); }
  iterator end() { return _elements.end(); }
private:
  std::vector<GASTNode*> _elements;
};

GASTNode* InitList(struct GASTNode* item);