#pragma once
#include "Plan.h"
#include <string>
#include "base/lang/ASTNode.h"
#include "base/Variant.h"

struct GListNode;
class GVirtualNetwork;
class GStorageEngine;
class GUtilPlan: public GPlan {
public:
  enum class UtilType {
    Creation,
    Drop,
    Dump,
  };
  GUtilPlan(std::map<std::string, GVirtualNetwork*>& vn, GStorageEngine* store, GCreateStmt* ast);
  GUtilPlan(std::map<std::string, GVirtualNetwork*>& vn, GStorageEngine* store, GDropStmt* ast);
  GUtilPlan(std::map<std::string, GVirtualNetwork*>& vn, GStorageEngine* store, GDumpStmt* ast);
  virtual int prepare();
  virtual int execute(const std::function<ExecuteStatus(KeyType, const std::string& key, nlohmann::json& value, int status)>&);

private:
private:
  UtilType _type;
  /**
   * @brief for creation, _var is graph name
   * 
   */
  Variant<std::string> _var;
  /**
   * @brief for creation, _vParams1 are groups name
   * 
   */
  std::vector<Variant<std::string>> _vParams1;
  /**
   * @brief for creation, _vParams2 are group's property
   * 
   */
  std::vector<std::vector<std::string>> _vParams2;
  /**
   * @brief for creation, _vParams3 are indexes
   * 
   */
  std::vector<Variant<std::string>> _vParams3;

};
