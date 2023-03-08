#include "base/lang/visitor/PlanVisitor.h"

VisitFlow GPlanVisitor::apply(GUpsetStmt* stmt, std::list<NodeType>& path) {
  return VisitFlow::SkipCurrent;

}
VisitFlow GPlanVisitor::apply(GCreateStmt* stmt, std::list<NodeType>& path) {
  return VisitFlow::SkipCurrent;

}
VisitFlow GPlanVisitor::apply(GDropStmt* stmt, std::list<NodeType>& path) {
  return VisitFlow::SkipCurrent;

}
VisitFlow GPlanVisitor::apply(GDumpStmt* stmt, std::list<NodeType>& path) {
  return VisitFlow::SkipCurrent;
}
VisitFlow GPlanVisitor::apply(GRemoveStmt* stmt, std::list<NodeType>& path) {
  return VisitFlow::SkipCurrent;

}