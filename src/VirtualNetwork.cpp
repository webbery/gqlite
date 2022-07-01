#include <future>
#include "VirtualNetwork.h"
#include "Graph/Node.h"

GVirtualNetwork::GVirtualNetwork(size_t maxMem)
:_current(nullptr)
, _maxMemory(maxMem)
{
}

void GVirtualNetwork::add(uint32_t id, GNode* node) {
  if (_ids.size() >= _maxMemory) {
    // detach nodes in future
    std::future<size_t> fut = std::async(&GVirtualNetwork::clean, this);
  }
  // add nodes
  _id2node[id] = node;
  _node2id[node] = id;
  _ids.push(id);
}

void GVirtualNetwork::release() {
  while (_ids.size() != 0) {
    uint32_t id = _ids.top();
    _ids.pop();
    GNode* p = _id2node[id];
    delete p;
  }
  _id2node.clear();
  _node2id.clear();
}

size_t GVirtualNetwork::clean() {
  uint32_t _id = _ids.top();
  _ids.pop();
  GNode* p = _id2node[_id];
  _id2node.erase(_id);
  _node2id.erase(p);
  delete p;
  return 0;
}