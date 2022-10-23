#pragma once
#include <list>
#include <queue>
#include "walk/WalkFactory.h"

#define NODE_CACHE_SIZE   16

namespace gql {
  template<typename _NodeKeyType>
  struct Order {
    std::list<_NodeKeyType> _ids;
    double _g;
    double _h;

    bool operator < (const Order& other) const {
      return _h < other._h;
    }
    bool operator > (const Order& other) const {
      return _h > other._h;
    }
  };

  using OrderQueue = std::priority_queue<Order<node_t>, std::vector<Order<node_t>>, std::greater<Order<node_t>>>;
  void stand(virtual_graph_t& vg, OrderQueue&, node_t id);
}

template<typename _Graph, typename Heuristic,
  typename Compare = std::greater<gql::Order<typename _Graph::node_t>>>
class GAStarWalk {
  using node_t = typename _Graph::node_t;
public:
  GAStarWalk(Heuristic& h)
    :_heuristic(h)
  {}

  void stand(_Graph& vg, node_t id = node_t()) {
    gql::stand(vg, _queue, id);
  }

  int walk(_Graph& vg, std::function<void(node_t, const node_info&)>) {
    if (vg.size() == 0) return WalkResult::WR_Preload;
    do
    {
      //if (vg.size() - _visited.size() < NODE_CACHE_SIZE) {
      //  return WalkResult::WR_Preload;
      //}
      auto current = _queue.top();
      if (_heuristic.success(current._ids)) return WalkResult::WR_Stop;
      _queue.pop();
      auto id_path = current._ids;
      auto last_id = id_path.back();
      _visited.insert(last_id);
      GMap<uint64_t, uint64_t>::node_collection curInfo;
      vg.visit(last_id, curInfo);
      double history = current._g;
      std::set<node_t> neighbors;
      if (!vg.neighbors(last_id, neighbors)) continue;
      for (node_t id : neighbors) {
        if (_visited.count(id)) {
          continue;
        }
        GMap<uint64_t, uint64_t>::node_collection collections;
        if (vg.visit(id, collections)) {
          auto d = _heuristic({ last_id, curInfo }, { id, collections });
          auto h = _heuristic.h(id);
          auto g = history + d;
          gql::Order<node_t> order{ id_path, g, g + h };
          order._ids.push_back(id);
          _queue.push(order);
        }
        else {
          beforeLoad();
          return WalkResult::WR_Preload | WalkResult::WR_UnVisit;
        }
        vg.neighbors(last_id, neighbors);
      }
    } while (vg.size() - _visited.size() > 0 && _queue.size());
    return 0;
  }
private:
  void beforeLoad() {}

private:
  Heuristic& _heuristic;
  /**
   * @brief order with >, the minest value will in the front.
   * 
   */
  std::priority_queue<gql::Order<node_t>, std::vector<gql::Order<node_t>>, Compare> _queue;
  /**
   * @brief record some node heuristic value. So if it's value is calculated, it will save in cache.
   */
  std::map<node_t, double> _heuristicCache;
  std::set<node_t> _visited;
};

template<typename Graph, typename Heuristic>
class IAStarWalkSelector {
  using node_t = typename Graph::node_t;
public:
  IAStarWalkSelector(Heuristic& h) : _walker(h), _pos(node_t()){}
  void stand(Graph& vg) { _walker.stand(vg, _pos); }

  int walk(Graph& vg, std::function<void(node_t, const node_info&)> f) {
    return _walker.walk(vg, f);
  }
protected:
  GAStarWalk<Graph, Heuristic > _walker;
  node_t _pos;
};

class IAStarHeuristic {
public:
  IAStarHeuristic(node_t target) :_target(target) {}

  bool success(const std::list<node_t>& l) {
    if (l.back() == _target) {
      _path = l;
      return true;
    }
    return false;
  }

protected:
  node_t _target;
  std::list<node_t> _path;
};