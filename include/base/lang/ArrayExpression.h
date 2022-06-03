#pragma once
#include <vector>

struct GASTNode;
class GArrayExpression {
public:
  GArrayExpression(GASTNode* elements, size_t len);

private:
  std::vector<GASTNode*> _elements;
};

GASTNode* InitList(struct GASTNode* item);