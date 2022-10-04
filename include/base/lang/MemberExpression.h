#pragma once
#include <string>

struct GASTNode;
class GMemberExpression {
public:
  GMemberExpression(GASTNode* obj, GASTNode* property, GASTNode* arguments = nullptr);
  ~GMemberExpression();

  std::string GetObjectName() const;
private:
  GASTNode* _obj;
  GASTNode* _prop;
  GASTNode* _args;
};