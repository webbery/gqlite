#include "walk/AStarWalk.h"

GAStarWalk::GAStarWalk(const std::string& prop)
  :_prop(prop)
{
}

void GAStarWalk::stand(virtual_graph_t& vg)
{
  auto beg = vg.nodes().begin();
  Order order = { beg->first, std::numeric_limits<double>::max() / 2 };
  _queue.push(order);
}

int GAStarWalk::walk(virtual_graph_t& vg, std::function<void(node_t, const node_info&)> f)
{
  if (vg.size() == 0) return WalkResult::WR_Preload;
  assert(_heuristic != nullptr);
  do
  {
    //if (vg.size() - _visited.size() < NODE_CACHE_SIZE) {
    //  return WalkResult::WR_Preload;
    //}
    auto current = _queue.top();
    _queue.pop();
    auto neighbors = vg.neighbors(current._id);
    GMap::node_collection curInfo;
    vg.visit(current._id, curInfo);
    for (node_t id : neighbors) {
      if (vg.is_visited(id)) continue;
      double value = 0;
      if (!_heuristicCache.count(id)) {
        GMap::node_collection collections;
        if (vg.visit(id, collections)) {
          value = _heuristic(curInfo, collections);
          _heuristicCache[id] = value;
          _visited.push_back(id);
          _queue.push({ id, value });
        }
        else {
          beforeLoad();
          return WalkResult::WR_Preload | WalkResult::WR_UnVisit;
        }
      }
      else {

      }
    }
  } while (vg.size() - _visited.size() > 0 && _queue.size());
  return 0;
}

void GAStarWalk::beforeLoad()
{

}

