#pragma once
#include "gqlite.h"
#include <string>

inline std::string replace_all(const std::string& input) {
  std::string data(input);
  size_t pos = 0;
  while ((pos = data.find("'", pos)) != std::string::npos) {
      data = data.replace(pos, 1, "\\'");
      pos += 2;
  }
  return data;
}

inline int gqlite_exec_callback(gqlite_result* params)
{
  if (params) {
    switch (params->type)
    {
    case gqlite_result_type_node:
    {
      gqlite_node* node = params->nodes;
      while (node) {
        switch (node->_type)
        {
        case gqlite_node_type_vertex:
        {
          gqlite_vertex* v = node->_vertex;
          printf("id: %s, properties: %s\n", v->id, v->properties);
        }
          break;
        case gqlite_node_type_edge:
          break;
        default:
          break;
        }
        node = node->_next;
      }
    }
      break;
    case gqlite_result_type_cmd:
      for (size_t idx = 0; idx < params->count; ++idx) {
        printf("%s", params->infos[idx]);
      }
      break;
    default:
      break;
    }
  }
  return 0;
}

inline int gqlite_cmd_callback(gqlite_result* params)
{
  //if (params && params->count) {
  //    for (size_t idx = 0; idx < params->count; ++idx) {
  //        printf("query result[%lu]: %s\n", idx, params->graphs[idx]);
  //    }
  //}
  return 0;
}