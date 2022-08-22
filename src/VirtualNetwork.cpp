#include <future>
#include "VirtualNetwork.h"
#include "Graph/Node.h"
#include "Graph/EntityNode.h"

GVirtualNetwork::GVirtualNetwork(size_t maxMem)
:_maxMemory(maxMem)
{
}

GVirtualNetwork::~GVirtualNetwork() {
  // if (_vg.size()) release();
  _event.join();
}

void GVirtualNetwork::addNode(node_t id, const std::vector<node_attr_t>& attr, const nlohmann::json& value) {
  if (_vg.size() >= _maxMemory) {
    // detach nodes in future
    std::future<size_t> fut = std::async(&GVirtualNetwork::clean, this);
    fut.get();
  }
  // add nodes
  GMap::node_attrs_t attrs(attr.begin(), attr.end());
  GMap::edges_t edges;
  GMap::node_cellection collection = std::make_tuple(edges, attrs, value);
  // cover if exist
  _vg._nodes[id] = collection;
}

void GVirtualNetwork::addEdge(edge_t id, node_t from, node_t to,
  const std::vector<node_attr_t>& attr, const nlohmann::json& value) {
  //
  assert(_vg._nodes.count(from));
  assert(_vg._nodes.count(to));

}

void GVirtualNetwork::release() {
  std::any arg = std::make_any<int>(0);
  _event.emit((int)VNMessage::WalkStop, arg);
  _vg.clear();
}

void GVirtualNetwork::startWalk() {
  std::any arg = std::make_any<int>(0);
  _event.emit((int)VNMessage::WalkStart, arg);
}

void GVirtualNetwork::stopWalk() {
  std::any arg = std::make_any<int>(0);
  _event.emit((int)VNMessage::WalkStop, arg);
}