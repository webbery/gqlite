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

inline int gqlite_exec_callback(gqlite_result* params, void*)
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
          if (v->type == gqlite_id_type::integer) {
            printf("[%lld, %s]\n", v->uid, v->properties);
          }
          else {
            printf("[%s, %s]\n", v->cid, v->properties);
          }
        }
          break;
        case gqlite_node_type_edge:
        {
          gqlite_edge* e = node->_edge;
          gqlite_vertex* f = e->from;
          gqlite_vertex* t = e->to;
          std::string out;
          if (f->type == gqlite_id_type::integer) {
            out += std::to_string(f->uid);
          }
          else {
            out += f->cid;
          }

          if (e->direction) {
            out += ", ->";
          }
          else {
            out += ", --";
          }

          if (e->properties) {
            out += ": " + std::string(e->properties, e->len);
          }

          out += ", ";
          if (t->type == gqlite_id_type::integer) {
            out += std::to_string(t->uid);
          }
          else {
            out += t->cid;
          }

          printf("[%s]\n", out.c_str());
        }
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
        printf("%s\n", params->infos[idx]);
      }
      break;
    default:
      break;
    }
  }
  return 0;
}

inline int gqlite_cmd_callback(gqlite_result* params, void*)
{
  if (params && params->count) {
    for (size_t idx = 0; idx < params->count; ++idx) {
      printf("[%lu]:\t%s\n", idx, params->infos[idx]);
    }
  }
  return 0;
}