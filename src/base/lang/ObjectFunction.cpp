#include "base/lang/ObjectFunction.h"
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
void GObjectFunction::addFunctionParam(GListNode* node) {
  _params.push_back(node);
}

void GObjectFunction::addFunctionParams(GListNode* node) {
  if (!node) return;
}

bool GObjectFunction::operator == (const GObjectFunction& other) {
  return _identify == other._identify;
}

GListNode* GObjectFunction::operator[](int index) {
  assert(index < _params.size());
  return _params[index];
}