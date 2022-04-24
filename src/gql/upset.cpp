#include "gql/upset.h"
#include "VirtualEngine.h"
#include "gql/query.h"
#include "base/type.h"
#include "base/ast.h"
#include "base/list.h"
#include "base/visitor.h"

namespace upset
{

  std::string exportVertexes(GGraphInstance* g)
  {
    // std::vector<VertexID> ids = GSinglecton::get<GStorageEngine>()->getNodes(g);
    // gqlite_result results;
    std::string gqline;
    // query::get_vertexes(g, ids, results);
    // gqlite_node* node = results.nodes;
    // while (node)
    // {
    //   switch (node->_type) {
    //   case gqlite_node_type_vertex:
    //     gqline += std::string("['") + node->_vertex->id + "'";
    //     if (node->_vertex->properties && node->_vertex->len) {
    //       gqline += "," + std::string(node->_vertex->properties, node->_vertex->len);
    //     }
    //     break;
    //   case gqlite_node_type_edge:
    //     break;
    //   default:
    //     break;
    //   }
    //   gqline += "]";
    //   node = node->_next;
    // }
    // if (results.nodes) gqline += "]}";
    return gqline;
  }

  std::string exportEdges(GGraphInstance* g)
  {
    std::string gqline;
    return gqline;
  }

} // namespace upset
