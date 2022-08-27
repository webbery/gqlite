#include <future>
#include "VirtualNetwork.h"
#include "Graph/Node.h"
#include "Graph/EntityNode.h"

GVirtualNetwork::GVirtualNetwork(size_t maxMem)
:_maxMemory(maxMem)
{
}

GVirtualNetwork::~GVirtualNetwork() {
   if (_vg.size()) release();
  //_event.join();
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
  GMap::node_cellection collection = std::make_tuple(edges, attrs, value);
  // cover if exist
  _vg._nodes[id] = collection;
  return 0;
}

int GVirtualNetwork::addEdge(edge_t id, node_t from, node_t to,
  const std::vector<node_attr_t>& attr, const nlohmann::json& value) {
  if (_event.is_finish()) return -1;
  //
  assert(_vg._nodes.count(from));
  assert(_vg._nodes.count(to));
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

void GVirtualNetwork::startWalk() {
  std::any arg = std::make_any<int>(0);
  _event.emit((int)VNMessage::WalkStart, arg);
}

void GVirtualNetwork::stopWalk() {
  std::any arg = std::make_any<int>(0);
  _event.emit((int)VNMessage::WalkStop, arg);
}