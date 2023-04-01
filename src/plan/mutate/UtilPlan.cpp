#include "plan/mutate/UtilPlan.h"
#include "Context.h"
#include "gqlite.h"
#include "VirtualNetwork.h"
#include "StorageEngine.h"
#include "base/lang/AST.h"
#include <fmt/printf.h>
#include "gutil.h"

#if __cplusplus > 201700
#include <filesystem>
using namespace std;
#elif __cplusplus > 201300
#include <experimental/filesystem>
using namespace std::experimental;
#elif __cplusplus > 201103 
#endif

#define CHECK_RETURN(expr) {\
  int ret = (expr);\
  if (ret != ECode_Success) return ret;\
}

GUtilPlan::GUtilPlan(GContext* context, GCreateStmt* stmt)
:GPlan(context->_graph, context->_storage, nullptr) {
    _type = UtilType::Creation;
    _var = stmt->name();
    GListNode* groups = stmt->groups();
    if (groups) {
      GArrayExpression* array = (GArrayExpression*)groups->_value;
      for (auto item: *array) {
        GGroupStmt* group = reinterpret_cast<GGroupStmt*>(item->_value);
        std::string name = group->name();
        GListNode* node = group->properties();
        if (node) {
          std::vector<std::string> props;
          GArrayExpression* array = reinterpret_cast<GArrayExpression*>(node->_value);
          for (auto prop: *array) {
            props.emplace_back(GetString(prop));
          }
          _vParams2.emplace_back(props);
        }
        GListNode* indexes = group->indexes();
        if (indexes) {
          GArrayExpression* array = (GArrayExpression*)indexes->_value;
          for (auto item : *array) {
            _vParams3.emplace_back(name + ":" + GetString(item));
          }
        }
        if (group->type() == GGroupStmt::Edge) {
          // edge group
          GEdgeGroupStmt* edgeGroup = static_cast<GEdgeGroupStmt*>(group);
          auto& schema = _store->getSchema();
          schema[SCHEMA_EDGE][edgeGroup->name()] = make_pair(edgeGroup->from(), edgeGroup->to());
        }
        _vParams1.emplace_back(name);
      }
    }
    
}

GUtilPlan::GUtilPlan(GContext* context, GDropStmt* stmt)
:GPlan(context->_graph, context->_storage, context->_schedule) {
  _type = UtilType::Drop;
  _var = stmt->name();
}

GUtilPlan::GUtilPlan(GContext* context, GDumpStmt* stmt)
:GPlan(context->_graph, context->_storage, context->_schedule)
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

int GUtilPlan::execute(GVM*, const std::function<ExecuteStatus(KeyType, const std::string& key, nlohmann::json& value, int status)>& processor) {
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
    if (filesystem::exists(graph)) {
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
    auto& edges = schema[SCHEMA_EDGE];
    auto indexes = _store->getIndexes();
    //fmt::printf("%s\n", schema.dump());
    // create graph
    std::string groupsName;
    for (auto itr = groups.begin(); itr != groups.end(); ++itr) {
      std::string name = itr.key();
      if (name == MAP_BASIC || name.find(":") != std::string::npos) continue;
      if (edges.find(name) != edges.end()) {
        auto& relation = edges[name];
        groupsName += std::string("['") + std::string(relation[0]) + "', '" + name + "', '" + std::string(relation[1]) + "'],";
      }
      else {
        // TODO: property will be consider if necessary
        groupsName += "{" + name + ": []";
        if (indexes.size()) {
          std::string sIndexes;
          std::string prefIndx(name + ":");
          for (auto& indx : indexes) {
            size_t pos = std::string(indx).find(prefIndx);
            if (pos != std::string::npos) {
              sIndexes += "'" + indx.substr(prefIndx.size(), indx.size() - prefIndx.size()) + "',";
            }
          }
          if (sIndexes.size()) {
            sIndexes.pop_back();
            sIndexes = ", index: [" + sIndexes + "]";
          }
          groupsName += sIndexes;
        }
        groupsName += "},";
      }
    }
    groupsName.pop_back();

    fmt::printf("{create: '%s', group: [%s]};\n", graph, groupsName);

    // upset group
    for (auto itr = groups.begin(); itr != groups.end(); ++itr) {
      std::string g = itr.key();
      if (g == MAP_BASIC || g.find(":") != std::string::npos) continue;
      KeyType type = (*itr)[SCHEMA_CLASS_KEY];
      std::function<std::string(const std::string&)> converter;
      if (type == KeyType::Byte) {
        converter = [](const std::string& s) {
          return "'" + s + "'";
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
          fmt::printf("{upset: '%s', vertex: [%s, %s]};\n", g, converter(key), gql::normalize(data));
        }
        else {
          fmt::printf("{upset: '%s', vertex: [%s]};\n", g, converter(key));
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
