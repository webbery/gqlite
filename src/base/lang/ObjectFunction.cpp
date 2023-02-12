#include "base/lang/ObjectFunction.h"
#include "base/lang/MemberExpression.h"
#include "base/lang/lang.h"

GObjectFunction::~GObjectFunction() {
  for (GListNode* param: _params) {
    FreeNode(param);
  }
  _params.clear();
}

void GObjectFunction::setFunctionName(const char* name, const char* scope) {
  _name = name;
  _scope = scope;
}

void GObjectFunction::setFunctionName(GListNode* node, const char* scope) {
  GMemberExpression* member = static_cast<GMemberExpression*>(node->_value);
  _name = member->GetObjectName() + "." + member->GetPropertyName();
  _scope = scope;
}

void GObjectFunction::addFunctionParam(GListNode* node) {
  _params.push_back(node);
}

void GObjectFunction::addFunctionParams(GListNode* node) {
  if (!node) return;
  if (node->_nodetype == NodeType::ArrayExpression) {
    GArrayExpression* arr = (GArrayExpression*)node->_value;
    for (int i = 0, end = arr->size(); i < end; ++i) {
      _params.push_back((*arr)[i]);
    }
    arr->clear();
  }
  else {
    printf("TODO: not implement for empty params\n");
  }
  FreeNode(node);
}

bool GObjectFunction::operator == (const GObjectFunction& other) {
  return _identify == other._identify;
}

GListNode* GObjectFunction::operator[](int index) {
  assert(index < _params.size());
  return _params[index];
}