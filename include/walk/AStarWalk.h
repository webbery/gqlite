#pragma once
#include <list>
#include <queue>
#include "walk/WalkFactory.h"

#define NODE_CACHE_SIZE   16

class GAStarWalk: public IWalkStrategy {
public:
  GAStarWalk(const std::string& prop);

  void setHeuristic(const std::function<double(const node_info& ,const node_info&)>& heuristic) {
    _heuristic = heuristic;
  }

  virtual void stand(virtual_graph_t& vg);
  virtual int walk(virtual_graph_t& vg, std::function<void(node_t, const node_info&)>);
private:
  struct Order {
    node_t _id;
    double _h;

    bool operator < (const Order& other) const {
      return _h < other._h;
    }
    bool operator > (const Order& other) const {
      return _h > other._h;
    }
  };

  void beforeLoad();

private:
  /**
   * @brief order with >, the minest value will in the front.
   * 
   */
  std::priority_queue<Order, std::vector<Order>, std::greater<Order>> _queue;
  /**
   * @brief record some node heuristic value. So if it's value is calculated, it will save in cache.
   */
  std::map<node_t, double> _heuristicCache;
  std::string _prop;
  std::function<double(const node_info& ,const node_info&)> _heuristic;
  std::list<node_t> _visited;
};