#include "base/lang/Property.h"
#include "base/lang/ASTNode.h"

GProperty::GProperty(const std::string& key, GASTNode* value)
: _key(key)
, _value(value) {

}

GProperty::~GProperty() {
  FreeAst(_value);
}