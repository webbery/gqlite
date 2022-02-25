#include "base/EdgeVisitor.h"
#include "Type/Edge.h"
#include "StorageEngine.h"
#include "Singlecton.h"

ASTEdgeUpdateVisitor::~ASTEdgeUpdateVisitor()
{

}

void ASTEdgeUpdateVisitor::visit(NodeType type, void* value)
{
  GEdgeStatment statement;
  statement.Parse((struct gast*)value);
  std::vector<uint8_t> data = statement.serialize();
  if (data.size() == 0) return;
  GGraph* pGraph = GSinglecton::get<GStorageEngine>()->getGraph();
  if (!pGraph)return;
}