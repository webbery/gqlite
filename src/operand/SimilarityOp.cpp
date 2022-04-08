#include "operand/SimilarityOp.h"
#include "SubGraph.h"
#include <queue>

namespace {}
/**
 * K. Riesen, S. Fankhauser, and H. Bunke. Speeding up graph edit distance computation with a bipartite heuristic
 */
float distance(GSubGraph* from, GSubGraph* to)
{
  // <vertex of from will be replaced, vertexes of to>
  std::deque<std::pair<std::string, std::string>> open;
  auto first = from->vertex_begin()->second;
  for (auto itr = to->vertex_begin(), end = to->vertex_end(); itr!=end; ++itr) {
    // insert substitude operation, first will be replaced by second
    open.push_back({first->id(), itr->second->id()});
  }
  // insert empty operation for deletion
  open.push_back({first->id(), std::string()});
  
  return 0;
}
