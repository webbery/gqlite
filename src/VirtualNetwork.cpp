#include <future>
#include "VirtualNetwork.h"
#include "Graph/Node.h"
#include "Graph/EntityNode.h"

namespace {
  template<typename T, typename Container = std::vector<T>>
  bool addUniqueDataAndSort(Container& pDest, T src) {
    auto ptr = std::lower_bound(pDest.begin(), pDest.end(), src);
    if (ptr == pDest.end() || *ptr != src) {
      pDest.insert(ptr, src);
      return false;
    }
    return true;  // exist
  }
}
GVirtualNetwork::GVirtualNetwork(size_t maxMem)
:_maxMemory(maxMem)
{
}

GVirtualNetwork::~GVirtualNetwork() {
  _event.join();
}

int GVirtualNetwork::addNode(node_t id, const std::vector<node_attr_t>& attr, const nlohmann::json& value) {
  if (_vg.size() >= _maxMemory) {
    // detach nodes in future
    std::future<size_t> fut = std::async(&GVirtualNetwork::clean, this);
    fut.get();
  }
  if (_event.is_finish()) return -1;
  // add nodes
  GMap::node_attrs_t attrs(attr.begin(), attr.end());
  GMap::edges_t edges;
  NodeStatus status = { false, false, false, NodeKind::Entity, 0 };
  GMap::node_collection collection = std::make_tuple(edges, attrs, value, status);
  // cover if exist
  _vg.nodes()[id] = collection;
  return 0;
}

int GVirtualNetwork::addEdge(edge_t id, node_t from, node_t to,
  const std::vector<node_attr_t>& attr, const nlohmann::json& value) {
  if (_event.is_finish()) return -1;
  //
  assert(_vg.nodes().count(from));
  assert(_vg.nodes().count(to));
  std::set<node_t> neighbors = _vg.neighbors(to);
  if (neighbors.count(from)) {
    auto& collection = _vg.edges()[id];
    std::get<3>(collection).direction = 0;
  }
  else {
    EdgeStatus status;
    status.hold = false;
    status.updated = false;
    status.kind = EdgeKind::Entity;
    status.direction = 0b10;
    parlay::sequence< GMap::edge_attr_t > attrs(attr.begin(), attr.end());
    GMap::edge_collection ec = std::make_tuple(std::pair<node_t, node_t>({ from, to }), attrs, value, status);
    _vg.edges()[id] = ec;

    auto& from_edges = std::get<0>(_vg.nodes()[from]);
    addUniqueDataAndSort< edge_t, parlay::sequence<edge_t> >(from_edges, id);
    auto& to_edges = std::get<0>(_vg.nodes()[to]);
    addUniqueDataAndSort< edge_t, parlay::sequence<edge_t> >(to_edges, id);
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

std::set<GMap::node_t> GVirtualNetwork::neighbors(node_t node)
{
  return _vg.neighbors(node);
}

void GVirtualNetwork::startWalk() {
  std::any arg = std::make_any<int>(0);
  _event.emit((int)VNMessage::WalkStart, arg);
}

void GVirtualNetwork::stopWalk() {
  std::any arg = std::make_any<int>(0);
  _event.emit((int)VNMessage::WalkStop, arg);
}