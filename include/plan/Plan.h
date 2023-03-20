#pragma once
#include "Context.h"
#include "gqlite.h"
#include <vector>
#include <string>
#include <functional>
#include "StorageEngine.h"

class GVirtualNetwork;
class GStorageEngine;
typedef int (*gqlite_callback)(_gqlite_result*, void*);

void init_result_info(gqlite_result& result, const std::vector<std::string>& info);
void release_result_info(gqlite_result& result);
void init_result_nodes(gqlite_result& result);
void release_result_nodes(gqlite_result& result);

/**
 * Control plan execute status.
 */
enum class ExecuteStatus {
  Continue,
  Stop
};

class GVM;
struct Chunk;
struct Compiler;
class GDefaultSchedule;
struct GContext;

class GPlan {
public:
  GPlan(GVirtualNetwork* networks, GStorageEngine* store, GDefaultSchedule* schedule);
  virtual ~GPlan();
  
  /** After Plan created, before Plan execute,
   * the prepare function will be called for validating preliminary.
   * For example: before update execute, we should check database created or not.
   */
  virtual int prepare() { return 0; }
  virtual int execute(GVM* gvm, const std::function<ExecuteStatus(KeyType, const std::string& key, nlohmann::json& value, int status)>& processor) = 0;
  /**
   * Try to interrupt plan when it still working
   */
  virtual int interrupt() { return 0; }
  void addLeft(GPlan* plan) { _left = plan; }

  inline GPlan* left() { return _left; }

  virtual void addCompiler(Compiler* compiler) { _compiler = compiler; }

protected:
  std::map<std::string, GVirtualNetwork*> _network;
  GStorageEngine* _store;
  GPlan* _left;
  GPlan* _right;
  Compiler* _compiler;
  GDefaultSchedule* _schedule;
};
