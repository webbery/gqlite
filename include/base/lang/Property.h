#pragma once
#include <string>

struct GListNode;
class GProperty {
public:
  GProperty(const std::string& key, GListNode* value);
  ~GProperty();

  std::string key() { return _key; }
  GListNode* value() { return _value; }
private:
  std::string _key;
  GListNode* _value;
};