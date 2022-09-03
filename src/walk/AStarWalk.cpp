#include "walk/AStarWalk.h"

namespace gql {
  void stand(virtual_graph_t& vg, std::priority_queue<Order, std::vector<Order>>& _queue, node_t id)
  {
    auto itr = vg.nodes().begin();
    if (id != (node_t)0) {
      //itr = 
    }
    Order order = { {itr->first}, std::numeric_limits<double>::max() / 2 };
    _queue.push(order);
  }

}
