#include <future>
#include "VirtualNetwork.h"
#include "Graph/Node.h"
#include "Graph/EntityNode.h"

GVirtualNetwork::GVirtualNetwork(size_t maxMem)
:_maxMemory(maxMem)
{
}

void GVirtualNetwork::addNode(uint32_t id) {
  if (_vg.size() >= _maxMemory) {
    // detach nodes in future
    std::future<size_t> fut = std::async(&GVirtualNetwork::clean, this);
  }
  // add nodes
  _vg._mNodesInstance[id] = new GEntityNode(_event);
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