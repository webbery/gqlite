#include "gql/upset.h"
#include "VirtualEngine.h"
#include "gql/query.h"
#include "base/type.h"
#include "base/ast.h"
#include "base/list.h"
#include "base/visitor.h"
#include "Singlecton.h"

namespace upset
{

  std::string exportVertexes(GGraph* g)
  {
    std::vector<VertexID> ids = GSinglecton::get<GStorageEngine>()->getNodes(g);
    gqlite_result results;
    query::get_vertexes(g, ids, results);
    std::string gqline;
    for (size_t idx = 0; idx < results.count; ++idx) {
      gqline += std::string("['") + results.nodes[idx].id + "'";
      if (results.nodes[idx].properties && results.nodes[idx].len) {
        gqline += "," + std::string(results.nodes[idx].properties, results.nodes[idx].len);
      }
      gqline += "]";
    }
    if (results.count) gqline += "]}";
    return gqline;
  }

  std::string exportEdges(GGraph* g)
  {
    std::string gqline;
    return gqline;
  }

} // namespace upset
