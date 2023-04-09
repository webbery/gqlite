/**
 * Test All Instruction in GVM
 */
#include <catch.hpp>
#include "base/gvm/GVM.h"
#include "StorageEngine.h"

TEST_CASE("simple graph api") {
  GStorageEngine engine;
  StoreOption opt;
  opt.compress = 1;
  opt.mode = ReadWriteOption::read_write;
  CHECK(engine.open("instructions.db", opt) == ECode_Success);
  GVM gvm(&engine);
}