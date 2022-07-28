#pragma once

struct GASTNode;
class GMemberExpression {
public:
  GMemberExpression(GASTNode* obj, GASTNode* property, GASTNode* arguments = nullptr);
  ~GMemberExpression();

private:
  GASTNode* _obj;
  GASTNode* _prop;
  GASTNode* _args;
};