#pragma once
#include <string>

struct GASTNode;
class GProperty {
public:
  GProperty(const std::string& key, GASTNode* value);

private:
  std::string _key;
  GASTNode* _value;
};