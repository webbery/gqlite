#pragma once
#include "Edge.h"
#include "base/type.h"
#include <string>

class EntityEdge : public GEdge {
public:

private:
  edge2_t _id;  // edge_t
  edge2_t _src_prev_eid;
  edge2_t _src_next_eid;
  edge2_t _dst_prev_eid;
  edge2_t _dst_next_eid;
};