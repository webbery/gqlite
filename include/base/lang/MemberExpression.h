#pragma once
#include <string>

struct GListNode;
class GMemberExpression {
public:
  GMemberExpression(GListNode* obj, GListNode* property, GListNode* arguments = nullptr);
  ~GMemberExpression();

  std::string GetObjectName() const;
  std::string GetPropertyName() const;
  
private:
  GListNode* _obj;
  GListNode* _prop;
  GListNode* _args;
};