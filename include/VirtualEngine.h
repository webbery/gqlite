#pragma once
#include "ParserEngine.h"
#include "StorageEngine.h"
#include <thread>
#include <mutex>
#include <condition_variable>

#define PROGRAM_Start   1
#define PROGRAM_Finish  2

class NoSQLRoutine {

};

class GStatement;
class AST;

enum OptimizerLevel {
  O_Normal,
  O_Speed
};

class GVirtualEngine{
public:
  GVirtualEngine();
  ~GVirtualEngine();

  void test(GStatement&);
  void addNoSQL(const NoSQLRoutine& instruction);
  void awaitExecute(NoSQLRoutine& instruction);
  void generateInternalRepresent(AST* pAst);

private:
  void execute();
  void interpret(NoSQLRoutine& i);
  void generateInstructions(OptimizerLevel level);

private:
  NoSQLRoutine _nosqls[512];
  int _top;
  std::thread _tInterpret;
  std::mutex _m;
  std::condition_variable _cv;
  void* _pParser;
};