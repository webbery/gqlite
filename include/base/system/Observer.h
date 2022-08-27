#pragma once
#include "json.hpp"

enum class KeyType: uint8_t;
class IObserver {
public:
  virtual ~IObserver() {}
  virtual void update(KeyType type, const std::string& key, const nlohmann::json& value) = 0;
};