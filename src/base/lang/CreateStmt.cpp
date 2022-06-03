#include "base/lang/CreateStmt.h"

GCreateStmt::GCreateStmt(const std::string& name, GASTNode* indexes)
: _name(name)
, _indexes(indexes) {}

std::string GCreateStmt::name() const { return _name; }

GASTNode* GCreateStmt::indexes() const { return _indexes; }