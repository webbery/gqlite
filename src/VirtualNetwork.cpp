#include "VirtualNetwork.h"
#include "Graph/Node.h"

GVirtualNetwork::GVirtualNetwork(size_t maxMem)
:_current(nullptr)
, _maxMemory(maxMem)
{
}

void GVirtualNetwork::add(GNode* node) {
}

void GVirtualNetwork::release() {}
