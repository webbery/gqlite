#pragma once
#include "Graph/Node.h"
#include <utility>
#include <stdint.h>
/**
 * Use for priority queue and support seach operation 
 */
class GHeap {
public:
  GHeap() {}
  ~GHeap() {}

  void push(uint32_t k, GNode* node) {}
  void pop() {}
  GNode* top() {}
  bool has(uint32_t ) { return true;}
  GNode* operator[](uint32_t) {}
  uint32_t size() { return 0; }

private:
  std::pair<uint32_t, GNode*>* _root;
};
