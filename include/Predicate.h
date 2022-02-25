#pragma once
#include <string>
#include <functional>

//typedef bool (*internal_predicate_func)(const std::any& left, const std::any& right);
//
//internal_predicate_func get_internal_predicate(const std::string& name);

template<typename T>
std::function<bool(T&, T&)> get_internal_predicate(const std::string& name) {
  if (name == "eq") {
    return [](T& left, T& right)->bool {return left == right; };
  }
  else if (name == "gt") {
    return [](T& left, T& right)->bool {return left > right; };
  }
  else if (name == "gte") {
    return [](T& left, T& right)->bool {return left >= right; };
  }
  else if (name == "lt") {
    return [](T& left, T& right)->bool {return left < right; };
  }
  else if (name == "lte") {
    return [](T& left, T& right)->bool {return left <= right; };
  }
  return nullptr;
}