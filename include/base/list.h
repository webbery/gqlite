#pragma once
#include "type.h"
#include <memory>

struct gql_node {
  void* _value;
  struct gql_node* _next;
};

template<typename T>
void release_callback_default(T& item) {}

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

template<typename Node>
Node* list_join(Node* first, Node* second) {
  if (!first) return second;
  if (!second) return first;
  Node* cur = first;
  while (cur->_next) cur = cur->_next;
  cur->_next = second;
  return first;
}

template <typename T>
struct GNode {
  GNode* _next;
};

// template<typename T>
// GNode<T>* init_list(GNode<T>* item) {
//   GNode<T>* node = (GNode<T>*)malloc(sizeof(GNode<T>));
//   // node->_value = item;
//   node->_next = nullptr;
//   return node;
// }