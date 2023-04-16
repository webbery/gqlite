#include "base/lang/visitor/WalkVisitor.h"
#include "base/lang/visitor/IVisitor.h"
#include "base/lang/lang.h"
#include "base/lang/AST.h"
#include "StorageEngine.h"
#include "VirtualNetwork.h"
#include "Graph/EntityEdge.h"
#include "Loader.h"
#include "walk/BSFWalk.h"

namespace gql {
  /**
  * For A* algorithm, args
  */
  int nativeBFSSearch(GVirtualNetwork* net, GScanPlan* plan) {
    /*class NodeVisitor {
    public:
      void operator()(node_t, const node_info&) {}
    };

    NodeVisitor visitor;
    GLoader* loader = new GLoader();
    GBFSHeuristic h((node_t)100);
    GBFSSelector<virtual_graph_t> selector(h);
    net->visit(selector, visitor, *loader);
    delete loader;
    delete net;*/
    return 0;
  }
}

GWalkVisitor::GWalkVisitor(GraphPattern* pattern)
  :_graph(pattern), _walkType(WalkType::EdgeQuery) {}

void GWalkVisitor::makeEdgeCondition(GWalkDeclaration::Order order, GEntityNode* start, GEntityNode* end, bool direction) {
  GEntityEdge* edge = new GEntityEdge(start, end);
  //if (order == GWalkDeclaration::VertexEdge) {
  //    edge->_start = start;
  //    edge->_end = end;
  //}
  //edge->_direction = direction;
  _graph->_edges.push_back(edge);
}
  
GEntityNode* GWalkVisitor::makeNodeCondition(const std::string& str) {
  GEntityNode* node = new GEntityNode((uint64_t)0);
  if (str != "*") {
    node->setProperty("label", const_cast<std::string&>(str));
  }
  return node;
}

VisitFlow GWalkVisitor::apply(GWalkDeclaration* walk, std::list<NodeType>& _) {
  auto order = walk->order();
  GEntityNode* node = nullptr;
  auto ptr = walk->root();
  while (ptr) {
    auto element = ptr->_element;
    if (element->_nodetype == EdgeDeclaration) {
      accept(element, this, _);
    }
    else {
      std::string str = GetString(element);
      int dir = 0;

      if (str == "--" || str == "->") {
        makeEdgeCondition(order, node, nullptr, (str == "--")? 0 : 1);
      }
      else if (str == "<-") {
        makeEdgeCondition(order, nullptr, node, 1);
      }
      else {
        node = makeNodeCondition(str);
        auto& edges = _graph->_edges;
        if (edges.size()) {
          auto& lastEdge = edges.back();
          if (order == GWalkDeclaration::VertexEdge) {
            if (lastEdge->to()) { lastEdge->setFrom(node); }
            else lastEdge->setTo(node);
          } else {
            printf("TODO: order for edge -> vertex.");
          }
        }
      }
    }
    ptr = ptr->_next;
  }
  return VisitFlow::SkipCurrent;
}

VisitFlow GWalkVisitor::apply(GEdgeDeclaration* stmt, std::list<NodeType>& path) {
  _walkType = WalkType::GraphMatch;
  if (stmt->from() && stmt->to()) {
    GEntityNode* start = makeNodeCondition(GetString(stmt->from()));
    GEntityNode* end = makeNodeCondition(GetString(stmt->to()));
    makeEdgeCondition(GWalkDeclaration::VertexEdge, start, end, stmt->direction() != "--");
    return VisitFlow::SkipCurrent;
  }
  else {
    return accept(stmt->value(), this, path);
  }
}

VisitFlow GWalkVisitor::apply(GObjectFunction*, std::list<NodeType>&) {
  return VisitFlow::SkipCurrent;
}
