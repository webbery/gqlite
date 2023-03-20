#include <catch.hpp>
#include "base/system/Coroutine.h"
#include "base/Promise.h"
#include "base/Future.h"
#include "schedule/DefaultSchedule.h"

TEST_CASE("simple coroutine") {
  GDefaultSchedule schedule(nullptr);
  schedule.addCoroutine([](GCoroutine* co) {
    for (int i = 1; i< 4; ++i) {
      co->yield();
      printf("%d\n", 2*i +1);
    }
    return 1;
  });
  schedule.addCoroutine([](GCoroutine* co) {
    for (int i = 1; i< 4; ++i) {
      co->yield();
      printf("%d\n", 2*(i + 1));
    }
      return 1;
  });
  schedule.run();
}

// extern "C" transfer_t jump_fcontext( fcontext_t const to, void * vp);

// extern "C" void* make_fcontext(void * sp, std::size_t size, void (* fn)(transfer_t));

// void func(transfer_t transfer) {
//     printf("I am in func.\n");
//     transfer = jump_fcontext(transfer.fctx, NULL);
//     printf("I am in func again!\n");
//     jump_fcontext(transfer.fctx, NULL);
// }

TEST_CASE("promise") {
  GDefaultSchedule schedule(nullptr);
  auto p = schedule.async([]() {
    printf("do work.\n");
    return 0;
  });
  //auto future = p.getFuture();
  //future.then([&p] (int v) {
  //  printf("value: %d\n", v);
  //  p.setValue(1);
  //}).then([]() {
  //   //printf("value next: %d\n", v);
  //   return 0;
  // }).get();
  schedule.run();
}