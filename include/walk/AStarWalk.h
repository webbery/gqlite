#pragma once
#include <list>
#include <queue>
#include "walk/WalkFactory.h"

#define NODE_CACHE_SIZE   16

namespace gql {
  struct Order {
    std::list<node_t> _ids;
    double _h;

    bool operator < (const Order& other) const {
      return _h < other._h;
    }
    bool operator > (const Order& other) const {
      return _h > other._h;
    }
  };

  void stand(virtual_graph_t& vg, std::priority_queue<Order, std::vector<Order>>&, node_t id);
}

template<typename Heuristuc, typename Compare = std::less<gql::Order>>
class GAStarWalk {
public:
  GAStarWalk(const std::string& prop, Heuristuc& h)
    :_prop(prop)
    , _heuristic(h)
  {}

  virtual void stand(virtual_graph_t& vg, node_t id = 0) {
    gql::stand(vg, _queue, id);
  }

  virtual int walk(virtual_graph_t& vg, std::function<void(node_t, const node_info&)>) {
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
      auto neighbors = vg.neighbors(id_path.back());
      GMap::node_collection curInfo;
      vg.visit(id_path.back(), curInfo);
      for (node_t id : neighbors) {
        if (vg.is_visited(id)) continue;
        double value = 0;
        if (!_heuristicCache.count(id)) {
          GMap::node_collection collections;
          if (vg.visit(id, collections)) {
            value = _heuristic(curInfo, collections);
            _heuristicCache[id] = value;
            _visited.push_back(id);
            gql::Order order{ id_path, value };
            order._ids.push_back(id);
            _queue.push(order);
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
private:
  void beforeLoad() {}

private:
  Heuristuc& _heuristic;
  /**
   * @brief order with >, the minest value will in the front.
   * 
   */
  std::priority_queue<gql::Order, std::vector<gql::Order>, Compare> _queue;
  /**
   * @brief record some node heuristic value. So if it's value is calculated, it will save in cache.
   */
  std::map<node_t, double> _heuristicCache;
  std::string _prop;
  std::list<node_t> _visited;
};

template<typename Heuristic>
class IAStarWalkSelector {
public:
  IAStarWalkSelector(const std::string& prop, Heuristic& h) : _walker(prop, h), _pos((node_t)0){}
  void stand(virtual_graph_t& vg) { _walker.stand(vg, _pos); }

  int walk(virtual_graph_t& vg, std::function<void(node_t, const node_info&)> f) {
    return _walker.walk(vg, f);
  }
protected:
  GAStarWalk< Heuristic > _walker;
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