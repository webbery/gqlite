#pragma once
#include "gqlite.h"

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