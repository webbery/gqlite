#include <cstdio>
#include <iostream>
#include <chrono>
#ifdef WIN32
#include <windows.h>
#include <conio.h>
#pragma warning(disable : 4054)
#pragma warning(disable : 4055)
#pragma warning(disable : 4100)
#pragma warning(disable : 4127)
#pragma warning(disable : 4130)
#pragma warning(disable : 4152)
#pragma warning(disable : 4189)
#pragma warning(disable : 4206)
#pragma warning(disable : 4210)
#pragma warning(disable : 4232)
#pragma warning(disable : 4244)
#pragma warning(disable : 4305)
#pragma warning(disable : 4306)
#pragma warning(disable : 4702)
#pragma warning(disable : 4706)
#pragma warning(disable : 4819)
#else
#include <unistd.h>
#include <termios.h>
#endif
#include <vector>
#include <string>
#include "gqlite.h"
#include "linenoise.h"
#include <fmt/color.h>
#include <fmt/printf.h>
#include "base/Debug.h"
#define MAX_HISTORY_SIZE  100
#define HISTORY_FILENAME  ".gql_history"

typedef int (*gqlite_display_result_callback)(gqlite_result*, void*);

bool g_dot_switch = false;
bool g_dot_query_start = false;

void trim(std::string& input) {
  if (input.empty()) return ;

  input.erase(0, input.find_first_not_of(" "));
  input.erase(input.find_last_not_of(" ") + 1);
}

void print_dot(const char* graph_name, gqlite_node* item) {

  while (item) {
    switch (item->_type) {
      case gqlite_node_type_vertex: {
        gqlite_vertex* v = item->_vertex;
          if (v->type == gqlite_id_type::integer) {
            printf("[%lld, %s]\n", v->uid, v->properties);
          }
          else {
            printf("[%s, %s]\n", v->cid, v->properties);
          }
      }
      break;
      case gqlite_node_type_edge: {
        if (!g_dot_query_start) {
          printf("digraph %s {\n", graph_name);
          g_dot_query_start = true;
        }
        gqlite_edge* e = item->_edge;
        gqlite_vertex* f = e->from;
        gqlite_vertex* t = e->to;
        std::string from, edge, to;
        if (f->type == gqlite_id_type::integer) {
          printf("  %lld", f->uid);
        }
        else {
          printf("  %s", f->cid);
        }
        if (e->direction) {
          printf(" -> ");
        } else {
          printf(" -- ");
        }
        if (t->type == gqlite_id_type::integer) {
          printf("%lld;\n", t->uid);
        }
        else {
          printf("%s;\n", t->cid);
        }
      }
      break;
    }
    item = item->_next;
  }
}

inline int query_exec_callback(gqlite_result* params, void*)
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
          std::string from, edge, to;
          if (f->type == gqlite_id_type::integer) {
            from += std::to_string(f->uid);
          }
          else {
            from += std::string("'") + f->cid + "'";
          }
          fmt::print(fmt::fg(fmt::color::indian_red), "{}, ", from);

          if (e->direction) {
            edge += "->";
          }
          else {
            edge += "--";
          }

          if (e->properties) {
            edge += ": " + std::string(e->properties, e->len);
          }
          fmt::print(fmt::fg(fmt::color::lime_green), "{}, ", edge);

          if (t->type == gqlite_id_type::integer) {
            to += std::to_string(t->uid);
          }
          else {
            to += std::string("'") + t->cid + "'";
          }

          fmt::print(fmt::fg(fmt::color::indian_red), "{}\n", to);
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

// show `dot` format of graph
int query_dot_result_callback(gqlite_result* params, void*) {
  if (params) {
    switch (params->type)
    {
    case gqlite_result_type_node: {
      gqlite_node* node = params->nodes;
      print_dot("G", node);
    }
    break;
    case gqlite_result_type_info:
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

int main(int argc, char** argv) {
#ifdef __linux__
  init_coredump_capture();
#elif _WIN32
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD dwMode = 0;
  GetConsoleMode(hOut, &dwMode);
  dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(hOut, dwMode);
  system("CHCP 65001 > nul 2>nul");
#endif
  std::string dbfile = "";
  if (argc > 1) {
    dbfile = argv[1];
  }
  linenoiseInstallWindowChangeHandler();
  linenoiseHistoryLoad(HISTORY_FILENAME);
  linenoiseSetCompletionCallback([](char const* prefix, linenoiseCompletions* lc) {

  });

  std::string input;
  gqlite* pHandle = 0;
  int ret = 0;
  if (dbfile.size()) {
    ret = gqlite_open(&pHandle, dbfile.c_str());
  } else {
    ret = gqlite_open(&pHandle);
  }
  if (ret) return ret;
  printf("GQLite Version %s\nWritting by Webberg.\n\nMIT License. Build: %s\n", PROJECT_VERSION, GIT_REVISION_SHA);
  gqlite_display_result_callback cb = query_exec_callback;
  do {
    char* result = linenoise("gqlite> ");
    if (result == nullptr) break;
    std::string input(result);
    free(result);
    trim(input);
    if (input == "exit") {
      break;
    }
    if (input == "dot on") {
      cb = query_dot_result_callback;
      g_dot_switch = true;
      continue;
    }
    else if (input == "dot off") {
      cb = query_exec_callback;
      g_dot_switch = false;
      continue;
    }
    linenoiseHistoryAdd(input.c_str());
    char* ptr = nullptr;
    auto start = std::chrono::high_resolution_clock::now();
    int error = gqlite_exec(pHandle, input.c_str(), cb, nullptr, &ptr);
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    if (g_dot_query_start) {
      printf("}\n");
      g_dot_query_start = false;
    }
    if (ptr) {
      auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() / 1000000.0;
      printf("%s, COST %gs\n", ptr, microseconds);
      gqlite_free(ptr);
    }
  } while (true);
  linenoiseHistorySave(HISTORY_FILENAME);
  linenoiseHistoryFree();
  gqlite_close(pHandle);
  return 0;
}