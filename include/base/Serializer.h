#pragma once
#include <vector>
#include <stdint.h>

class GSerializer {
public:
  virtual ~GSerializer() {}
  virtual std::vector<uint8_t> serialize() = 0;
  virtual void deserialize(uint8_t* data, size_t len) = 0;
};