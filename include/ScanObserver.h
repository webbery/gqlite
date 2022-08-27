#pragma once
#include "base/system/Observer.h"

class GScanObserver: public IObserver {
public:
  virtual void update(KeyType type, const std::string& key, const nlohmann::json& value);
};