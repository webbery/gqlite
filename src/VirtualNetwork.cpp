#include <future>
#include "VirtualNetwork.h"
#include "Graph/Node.h"
#include "Graph/EntityNode.h"

GVirtualNetwork::GVirtualNetwork(size_t maxMem)
:_maxMemory(maxMem)
{
}

GVirtualNetwork::~GVirtualNetwork() {
  _event.join();
}

int GVirtualNetwork::addNode(node_t id, const std::vector<node_attr_t>& attr, const nlohmann::json& value, uint8_t level) {
  if (_vg.size() >= _maxMemory) {
    // detach nodes in future
    std::future<size_t> fut = std::async(&GVirtualNetwork::clean, this);
    fut.get();
  }
  if (_event.is_finish()) return -1;
  // add nodes
  auto& nodes = _vg.nodes();
  if (nodes.count(id)) {
    auto& s = std::get<3>(nodes[id]);
    if (s.layer > level) return 0;
  }
  GMap::node_attrs_t attrs(attr.begin(), attr.end());
  std::array< GMap::edges_t, MAX_LAYER_SIZE> edges;
  NodeStatus status = { false, false, false, NodeKind::Entity, level };
  GMap::node_collection collection = std::make_tuple(edges, attrs, value, status);
  // cover if exist
  nodes[id] = collection;
  return 0;
}

int GVirtualNetwork::addEdge(edge_t id, node_t from, node_t to,
  const std::vector<node_attr_t>& attr, const nlohmann::json& value, int8_t level) {
  if (_event.is_finish()) return -1;
  //
  assert(_vg.nodes().count(from));
  assert(_vg.nodes().count(to));
  std::set<node_t> neighbors;
  if (_vg.neighbors(to, neighbors, level) && neighbors.count(from)) {
    if (_vg.edges().count(id) == 0) return -1;
    auto& collection = _vg.edges()[id];
    std::get<3>(collection).direction = 0;
  }
  else {
    EdgeStatus status;
    status.hold = false;
    status.updated = false;
    status.del = false;
    status.direction = 0b10;
    status.layer = level;
    parlay::sequence< GMap::edge_attr_t > attrs(attr.begin(), attr.end());
    GMap::edge_collection ec = std::make_tuple(std::pair<node_t, node_t>({ from, to }), attrs, value, status);
    _vg.edges()[id] = ec;

    auto& from_edges = std::get<0>(_vg.nodes()[from]);
    addUniqueDataAndSort< edge_t, parlay::sequence<edge_t> >(from_edges[level], id);
    auto& to_edges = std::get<0>(_vg.nodes()[to]);
    addUniqueDataAndSort< edge_t, parlay::sequence<edge_t> >(to_edges[level], id);
  }
  return 0;
}

int GVirtualNetwork::deleteEdge(edge_t id, bool mark)
{
  if (mark) {
    std::get<3>(_vg.edges()[id]).del = true;
  }
  else {
    std::get<3>(_vg.edges()[id]).hold = true;
    auto eraseEdgeFrom = [this, id](node_t node) {
      auto& fn = std::get<0>(_vg.nodes()[node]);
      for (auto itr = fn.begin(); itr != fn.end(); ++itr)
      {
        //if (*itr == id) {
        //  fn.erase(itr);
        //  break;
        //}
      }
    };
    auto& nodes = std::get<0>(_vg.edges()[id]);
    eraseEdgeFrom(nodes.first);
    eraseEdgeFrom(nodes.second);
    _vg.edges().erase(id);
  }
  return 0;
}

void GVirtualNetwork::join()
{
  _event.join();
}

void GVirtualNetwork::release() {
  std::any arg = std::make_any<int>(0);
  _event.emit((int)VNMessage::WalkStop, arg);
  _event.finish();
  _event.join();
}

bool GVirtualNetwork::neighbors(node_t node, std::set<node_t>& n, int8_t level)
{
  return _vg.neighbors(node, n, level);
}

size_t GVirtualNetwork::node_size()
{
  return _vg.node_size();
}

node_const_iterator GVirtualNetwork::node_begin() const
{
  return _vg.node_begin();
}

node_const_iterator GVirtualNetwork::node_end() const
{
  return _vg.node_end();
}
