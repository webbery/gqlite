#include "VirtualNetwork.h"

GVirtualNetwork::GVirtualNetwork(size_t maxMem)
:_maxMemory(maxMem)
,_current(nullptr) {}

void GVirtualNetwork::addNode(GNode* node) {

}

void GVirtualNetwork::release() {}