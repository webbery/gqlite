#pragma once
#include "type.h"
#include "ast.h"
#include <memory>

struct gql_node {
  void* _value;
  struct gql_node* _next;
};

template<typename T>
void release_callback_default(T& item) {}

template<typename T>
gql_node* init_list(T& item, NodeType type) {
  gql_node* node = (gql_node*)malloc(sizeof(gql_node));
  node->_value = malloc(sizeof(T));
  memcpy(node->_value, &item, sizeof(T));
  node->_next = nullptr;
  return node;
}

inline gql_node* init_list(struct gast* item) {
  gql_node* node = (gql_node*)malloc(sizeof(gql_node));
  node->_value = item;
  node->_next = nullptr;
  return node;
}

template<typename Fn>
void release_list(gql_node* p, Fn cb = release_callback_default) {
  if (!p) return;
  gql_node* cur = p;
  while(!cur) {
    release_callback_default(cur->_value);
    gql_node* temp = cur;
    cur = cur->_next;
    free(temp->_value);
    free(temp);
  }
}

inline gql_node* list_join(struct gql_node* first, struct gql_node* second) {
  if (!first) return second;
  if (!second) return first;
  gql_node* cur = first;
  while (cur->_next) cur = cur->_next;
  cur->_next = second;
  return first;
}