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

void GVirtualNetwork::release() {
  
}

size_t GVirtualNetwork::clean() {
  
  return 0;
}

const std::vector<std::string>& GVirtualNetwork::attributes() const {
  return _attributes;
}