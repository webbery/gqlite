#pragma once
#include <string>

struct GASTNode;
class GProperty {
public:
  GProperty(const std::string& key, GASTNode* value);

  std::string key() { return _key; }
  GASTNode* value() { return _value; }
private:
  std::string _key;
  GASTNode* _value;
};