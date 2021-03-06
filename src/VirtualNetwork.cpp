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

void GVirtualNetwork::addNode(uint32_t id, const std::vector<node_attr_t>& attr, const std::vector<node_literal_t>& value) {
  if (_vg.size() >= _maxMemory) {
    // detach nodes in future
    std::future<size_t> fut = std::async(&GVirtualNetwork::clean, this);
  }
  // add nodes
  GMap::node_cellection collection;
  if (_vg._nodes.count(id)) {

  }
  // _vg._nodes[id] = 
}

void GVirtualNetwork::addEdge(uint32_t id) {}

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