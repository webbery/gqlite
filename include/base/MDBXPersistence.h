#pragma once
#include "Persistence.h"

class MDBXPersistence : public GPersistence {
public:
  virtual int Init(const char* filename);
};