#include "VirtualNetwork.h"
#include "Graph/Node.h"

GVirtualNetwork::GVirtualNetwork(size_t maxMem)
:_current(nullptr)
,_cache(maxMem)
{
}

void GVirtualNetwork::add(GNode* node) {
  
}

void GVirtualNetwork::release() {}