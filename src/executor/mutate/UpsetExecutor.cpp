#include "executor/mutate/UpsetExecutor.h"
#include "executor/Executor.h"

GUpsetExecutor::~GUpsetExecutor() {

}
int GUpsetExecutor::open() {
  return 0;
}
std::future<GExecuteResult> GUpsetExecutor::execute(GContext* ctx) {
  return std::future<GExecuteResult>();
}
int GUpsetExecutor::close() {
  return 0;
}