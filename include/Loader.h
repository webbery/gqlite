#pragma once
#include "VirtualNetwork.h"

class GScanPlan;
class GLoader {
public:
  GLoader();

  bool load() const;
private:
  /**
  * For byte id, it give an convert map. 
  * If an data loaded, a new node id will generate and put it to _keyMap.
  * This id is simple add 1 by _maxID.
  */
  std::map<GVirtualNetwork::node_t, std::string> _keyMap;
  GVirtualNetwork::node_t _maxID;

};