#include "plan/UtilPlan.h"
#include "gqlite.h"
#include "VirtualNetwork.h"
#include "StorageEngine.h"
#include "base/lang/AST.h"
#include <filesystem>
#include <fmt/printf.h>
#include "gutil.h"

#define CHECK_RETURN(expr) {\
  int ret = (expr);\
  if (ret != ECode_Success) return ret;\
}

GUtilPlan::GUtilPlan(std::map<std::string, GVirtualNetwork*>& vn, GStorageEngine* store, GCreateStmt* stmt)
:GPlan(vn, store) {
    _type = UtilType::Creation;
    _var = stmt->name();
    GASTNode* groups = stmt->groups();
    if (groups) {
      GArrayExpression* array = (GArrayExpression*)groups->_value;
      for (auto item: *array) {
        GGroupStmt* stmt = reinterpret_cast<GGroupStmt*>(item->_value);
        std::string name = stmt->name();
        GASTNode* node = stmt->properties();
        if (node) {
          std::vector<std::string> props;
          GArrayExpression* array = reinterpret_cast<GArrayExpression*>(node->_value);
          for (auto prop: *array) {
            props.emplace_back(GetString(prop));
          }
          _vParams2.emplace_back(props);
        }
        _vParams1.emplace_back(name);
      }
    }
    GASTNode* indexes = stmt->indexes();
    if (indexes) {
      GArrayExpression* array = (GArrayExpression*)indexes->_value;
      for (auto item : *array) {
        _vParams3.emplace_back(GetString(item));
      }
    }
}

GUtilPlan::GUtilPlan(std::map<std::string, GVirtualNetwork*>& vn, GStorageEngine* store, GDropStmt* stmt)
:GPlan(vn, store) {
  _type = UtilType::Drop;
  _var = stmt->name();
}

GUtilPlan::GUtilPlan(std::map<std::string, GVirtualNetwork*>& vn, GStorageEngine* store, GDumpStmt* stmt)
  : GPlan(vn, store)
{
  _type = UtilType::Dump;
  _var = stmt->name();
}

int GUtilPlan::prepare()
{
  int ret = ECode_Success;
  switch (_type)
  {
  case GUtilPlan::UtilType::Creation:
    break;
  case GUtilPlan::UtilType::Drop:
    break;
  case GUtilPlan::UtilType::Dump:
    break;
  default:
    break;
  }
  return ret;
}

int GUtilPlan::execute(const std::function<ExecuteStatus(KeyType, const std::string& key, const std::string& value)>& processor) {
  switch (_type)
  {
  case UtilType::Creation:
  {
    if (!_store) return ECode_DISK_OPEN_FAIL;
    StoreOption opt;
    opt.compress = 1;
    CHECK_RETURN(_store->open(std::get<std::string>(_var).c_str(), opt));
    for (auto& item : _vParams1) {
      std::string v = std::get<std::string>(item);
      //printf("add map: %s\n", v.c_str());
      _store->addMap(v, KeyType::Uninitialize);
    }
    for (auto& item : _vParams3) {
      std::string v = std::get<std::string>(item);
      //printf("add index: %s\n", v.c_str());
      _store->addIndex(v);
    }
  }
    break;
  case UtilType::Drop:
  {
    std::string graph = std::get<std::string>(_var);
    _store->close();
    if (std::filesystem::exists(graph)) {
      if (std::remove(graph.c_str())) {
        perror("drop");
        return ECode_DB_Drop_Fail;
      }
    }
    else {
      return ECode_Graph_Not_Exist;
    }
  }
    break;
  case UtilType::Dump:
  {
    std::string graph = std::get<std::string>(_var);
    auto& schema = _store->getSchema();
    auto& groups = schema[SCHEMA_CLASS];
    // create graph
    std::string groupsName;
    for (auto itr = groups.begin(); itr != groups.end(); ++itr) {
      std::string name = itr.key();
      if (name == MAP_BASIC || name.find(":") != std::string::npos) continue;
      groupsName += "'" +name + "'";
      groupsName += ",";
    }
    groupsName.pop_back();
    auto indexes = _store->getIndexes();
    if (indexes.size()) {
      std::string sIndexes;
      for (auto& indx : indexes) {
        sIndexes += "'" + indx + "',";
      }
      sIndexes.pop_back();
      fmt::printf("{create: '%s', group: [%s], index: [%s]}\n", graph, groupsName, sIndexes);
    }
    else {
      fmt::printf("{create: '%s', group: [%s]}\n", graph, groupsName);
    }
    // upset group
    for (auto itr = groups.begin(); itr != groups.end(); ++itr) {
      std::string g = itr.key();
      if (g == MAP_BASIC || g.find(":") != std::string::npos) continue;
      KeyType type = (*itr)[SCHEMA_CLASS_KEY];
      std::function<std::string(const std::string&)> converter;
      if (type == KeyType::Byte) {
        converter = [](const std::string& s) {
          return s;
        };
      }
      else if (type == KeyType::Integer) {
        converter = [](const std::string& s) {
          uint64_t val = *(uint64_t*)s.data();
          return std::to_string(val);
        };
      }
      else {
        continue;
      }
      auto cursor = _store->getMapCursor(g);
      auto result = cursor.to_first(false);
      while (result)
      {
        std::string data((char*)result.value.byte_ptr(), result.value.size());
        std::string key((char*)result.key.byte_ptr(), result.key.size());
        if (data != "null") {
          fmt::printf("{upset: '%s', vertex: [%s, %s]}\n", g, converter(key), gql::normalize(data));
        }
        else {
          fmt::printf("{upset: '%s', vertex: [%s]}\n", g, converter(key));
        }
        result = cursor.to_next(false);
      }
    }
  }
    break;
  default:
    break;
  }
  return ECode_Success;
}
