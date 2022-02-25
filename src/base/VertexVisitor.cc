#include "base/VertexVisitor.h"
#include "Type/Vertex.h"
#include "VirtualEngine.h"
#include "StorageEngine.h"
#include "Singlecton.h"

ASTVertexUpdateVisitor::~ASTVertexUpdateVisitor()
{

}

void ASTVertexUpdateVisitor::visit(NodeType type, void* value)
{
  GVertexStatment statement;
  statement.Parse((struct gast*)value);
  std::vector<uint8_t> data = statement.serialize();
  if (data.size() == 0) return;
  GGraph* pGraph = GSinglecton::get<GStorageEngine>()->getGraph();
  if (!pGraph)return;
  pGraph->updateVertex(statement.id(), data);
  // update indexes
  const GraphProperty& props = pGraph->property();
  for (const std::string& indx: props._indexes)
  {
    pGraph->updateIndex(statement.id(), indx, statement.value(indx));
  }
}

ASTVertexQueryVisitor::~ASTVertexQueryVisitor()
{

}

void ASTVertexQueryVisitor::visit(NodeType, void* value)
{
  GQueryConditionClause clause;
  clause.Parse((gast*)value);
  _preds = clause._preds;
}
