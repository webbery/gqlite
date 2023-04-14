/**
 * Test For Frog framework
 */
#include <catch.hpp>
#include "StorageEngine.h"
 //#include "schedule/FrogSchedule.h"

TEST_CASE("bsf for cpu") {
  GStorageEngine engine;
  StoreOption opt;
  opt.compress = 1;
  opt.mode = ReadWriteOption::read_write;
  CHECK(engine.open("./basketballplayer", opt) == ECode_Success);


}
