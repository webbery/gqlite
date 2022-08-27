#pragma once
#include "base/system/Observer.h"
#include "Loader.h"

class GScanObserver: public IObserver {
public:
  GScanObserver(GLoader* loader);
  virtual ~GScanObserver();
  virtual void update(KeyType type, const std::string& key, const nlohmann::json& value);

private:
  GLoader* _loader;
};