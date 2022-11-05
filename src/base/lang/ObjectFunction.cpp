#include "base/lang/ObjectFunction.h"
#include "base/lang/lang.h"

GObjectFunction::~GObjectFunction() {
  for (GASTNode* param: _params) {
    FreeAst(param);
  }
}

void GObjectFunction::setFunctionName(const char* name, const char* scope) {
  _name = name;
  _scope = scope;
}
void GObjectFunction::addFunctionParam(GASTNode* node) {
  _params.push_back(node);
}

bool GObjectFunction::operator == (const GObjectFunction& other) {
  return _identify == other._identify;
}

GASTNode* GObjectFunction::operator[](int index) {
  assert(index < _params.size());
  return _params[index];
}