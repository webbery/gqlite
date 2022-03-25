#pragma once
#include "type.h"
#include "json.hpp"
#include "list.h"

struct gast {
  enum NodeType _nodetype;
  void* _value;
  struct gast* _left;
  struct gast* _right;
};

struct gast* newast(enum NodeType type, void* value, struct gast* left, struct gast* right);
void freeast(struct gast* root);
void dumpast(struct gast* root, int level = 0);
struct gast* loadast(const char* filename);

struct gql_node* as_array(struct gast* root);

class ASTVisitor;
void traverse(struct gast*, ASTVisitor* visitor);

double GET_NUMBER_VALUE(struct gast* ast);
int32_t GET_INT_VALUE(struct gast* ast);
std::string GET_STRING_VALUE(struct gast* ast);

template<NodeType type> class Acceptor {};

class ArrayVisitor {
public:
  template<typename Acceptor>
  void visit(Acceptor& acceptor) {
    _value.push_back(acceptor.value());
  }

  nlohmann::json value() { return _value; }
private:
  nlohmann::json _value;
};

class PropertyVisitor {
public:
  PropertyVisitor(const std::string& name)
  :_name(name){
  }

  template<typename Acceptor>
  void visit(Acceptor& acceptor) {
    _value[_name] = acceptor.value();
    // printf("\tname: %s, value: %s\n", _name.c_str(), _value[_name].dump().c_str());
  }

  nlohmann::json value() { return _value; }
private:
  std::string _name;
  nlohmann::json _value;
};

class ObjectVisitor {
public:
  void visit(Acceptor<NodeType::Property>& acceptor);
  void visit(Acceptor<NodeType::Number>& acceptor);
  void visit(Acceptor<NodeType::Integer>& acceptor);
  void visit(Acceptor<NodeType::ArrayExpression>& acceptor);
  void visit(Acceptor<NodeType::String>& acceptor);
  void visit(Acceptor<NodeType::ObjectExpression>& acceptor);
  nlohmann::json value() { return _value; }
private:
  nlohmann::json _value;
};

template<> class Acceptor<NodeType::Property> {
public:
  Acceptor(PropertyVisitor& pv)
  :_value(pv.value()){}

  template<typename Visitor>
  void accept(Visitor& visitor) {
    visitor.visit(*this);
  }
  nlohmann::json value() { return _value; }
private:
  nlohmann::json _value;
};

template<> class Acceptor<NodeType::String> {
public:
  Acceptor(const std::string& value) :_value(value) {}

  template<typename Visitor>
  void accept(Visitor& visitor) {
    visitor.visit(*this);
  }

  std::string value() { return _value; }
private:
  std::string _value;
};

template<> class Acceptor<NodeType::Number> {
public:
  Acceptor(double value) :_value(value) {}

  template<typename Visitor>
  void accept(Visitor& visitor) {
    visitor.visit(*this);
  }

  double value() { return _value; }
private:
  double _value;
};

template<> class Acceptor<NodeType::Integer> {
public:
  Acceptor(double value) :_value(value) {}

  template<typename Visitor>
  void accept(Visitor& visitor) {
    visitor.visit(*this);
  }

  int32_t value() { return _value; }
private:
  int32_t _value;
};

template<> class Acceptor<NodeType::ArrayExpression> {
public:
  Acceptor(ArrayVisitor& av) {
    this->_value = av.value();
  }

  template<typename Visitor>
  void accept(Visitor& visitor) {
    visitor.visit(*this);
  }
  nlohmann::json value() { return _value; }
private:
  nlohmann::json _value;
};

template<> class Acceptor<NodeType::ObjectExpression> {
public:
  Acceptor(ObjectVisitor& ov):_value(ov.value()) {}

  template<typename Visitor>
  void accept(Visitor& visitor) {
    visitor.visit(*this);
  }
  nlohmann::json value() { return _value; }
private:
  nlohmann::json _value;
};

template<typename Visitor>
void GET_VALUE(struct gast* ast, bool& hasBinary, Visitor& visitor) {
  if (!ast) return;
  // printf("\tType: %d", ast->_nodetype);
  switch (ast->_nodetype)
  {
  case NodeType::Property:
  {
    std::string k = GET_STRING_VALUE(ast->_left);
    PropertyVisitor pv(k);
    GET_VALUE(ast->_right, hasBinary, pv);
    Acceptor< NodeType::Property > acceptor(pv);
    acceptor.accept(visitor);
  }
    break;
  case NodeType::String:
  {
    std::string value = GET_STRING_VALUE(ast);
    Acceptor< NodeType::String > acceptor(value);
    acceptor.accept(visitor);
  }
    break;
  case NodeType::Number:
  {
    double value = GET_NUMBER_VALUE(ast);
    Acceptor< NodeType::Number > acceptor(value);
    acceptor.accept(visitor);
  }
    break;
  case NodeType::Integer:
  {
    int32_t value = GET_INT_VALUE(ast);
    Acceptor< NodeType::Integer > acceptor(value);
    acceptor.accept(visitor);
  }
    break;
  case NodeType::Binary:
    hasBinary = true;
    break;
  case NodeType::ArrayExpression:
  {
    gql_node* node = (gql_node*)ast->_value;
    if (!node) break;
    ArrayVisitor av;
    while (node)
    {
      GET_VALUE((gast*)node->_value, hasBinary, av);
      node = node->_next;
    }
    // printf("ARRAY: %s\n", av.value().dump().c_str());
    Acceptor<NodeType::ArrayExpression> acceptor(av);
    acceptor.accept(visitor);
  }
    break;
  case NodeType::ObjectExpression:
  {
    gast* node = (gast*)ast->_value;
    ObjectVisitor ov;
    GET_VALUE(node, hasBinary, ov);
    Acceptor<NodeType::ObjectExpression> acceptor(ov);
    acceptor.accept(visitor);
  }
    break;
  default:
    break;
  }
}
