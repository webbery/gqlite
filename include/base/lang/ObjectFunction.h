#pragma once
#include "./ObjectStmt.h"
#include <string>
#include <vector>

class GObjectFunction : public GObjectStmt {
public:
  using Params = std::vector<GListNode*>;
  using iterator = Params::iterator;
  using const_iterator = Params::const_iterator;

  ~GObjectFunction();
  
  void setFunctionName(const char* name, const char* scope);
  void addFunctionParam(GListNode* node);
  void addFunctionParams(GListNode* node);

  std::string scope() { return _scope; }
  std::string name() { return _name; }

  bool operator == (const GObjectFunction& other);
  
  const_iterator params_begin() const { return _params.begin(); }
  iterator params_begin() { return _params.begin(); }

  const_iterator params_end() const { return _params.end(); }
  iterator params_end() { return _params.end(); }

  GListNode* operator[](int index);

private:
  std::string _name;
  std::string _scope;
  std::string _identify;

  std::vector<GListNode*> _params;
};