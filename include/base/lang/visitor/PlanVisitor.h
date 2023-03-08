#pragma once

#include "base/lang/visitor/IVisitor.h"

/**
 * Visit some element and generate plan node
 */
class GPlanVisitor : public GVisitor {
public:
  VisitFlow apply(GUpsetStmt* stmt, std::list<NodeType>& path);
  VisitFlow apply(GCreateStmt* stmt, std::list<NodeType>& path);
  VisitFlow apply(GDropStmt* stmt, std::list<NodeType>& path);
  VisitFlow apply(GDumpStmt* stmt, std::list<NodeType>& path);
  VisitFlow apply(GRemoveStmt* stmt, std::list<NodeType>& path);
};