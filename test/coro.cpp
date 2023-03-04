#include <catch.hpp>
#include "base/system/Coroutine.h"

struct Test{
  Test(GCoSchedule* s): _schedule(s) {}

  void work() {
    _schedule->createCoroutine([&](GCoroutine* co) {
      printf("1\n");
      for (int i = 1; i< 4; ++i) {
        co->yield();
        printf("%d\n", 2*i +1);
      }
    });
    _schedule->createCoroutine([](GCoroutine* co) {
      printf("2\n");
      for (int i = 1; i< 4; ++i) {
        co->yield();
        printf("%d\n", 2*(i + 1));
      }
    });
  }

  GCoSchedule* _schedule;
};

TEST_CASE("simple coroutine") {
  GCoSchedule schedule;
  schedule.createCoroutine([](GCoroutine* co) {
    for (int i = 1; i< 4; ++i) {
      co->yield();
      printf("%d\n", 2*i +1);
    }
  });
  schedule.createCoroutine([](GCoroutine* co) {
    for (int i = 1; i< 4; ++i) {
      co->yield();
      printf("%d\n", 2*(i + 1));
    }
  });
  schedule.run();
  schedule.join();
}

TEST_CASE("class coroutine") {
}