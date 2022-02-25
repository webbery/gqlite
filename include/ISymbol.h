#pragma once
#if (__cplusplus >= 201703L)
#include <any>
typedef std::any any;
#define Any_cast  std::any_cast
//namespace gql = std;
#else (__cplusplus > 201103L)
#include <experimental/any>
typedef std::experimental::any any;
#define Any_cast  std::experimental::any_cast
#endif
#include "json.hpp"

namespace gql {
  struct vertex {
    std::string _id;
    nlohmann::json* _json;
  };

  struct edge {
    VertexID _from;
    VertexID _to;
    bool _bidirection;
  };
  
}
