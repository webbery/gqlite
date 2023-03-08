#include <catch.hpp>
#include "base/system/Coroutine.h"
#include "base/system/context.h"

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

// extern "C" transfer_t jump_fcontext( fcontext_t const to, void * vp);

// extern "C" void* make_fcontext(void * sp, std::size_t size, void (* fn)(transfer_t));

// void func(transfer_t transfer) {
//     printf("I am in func.\n");
//     transfer = jump_fcontext(transfer.fctx, NULL);
//     printf("I am in func again!\n");
//     jump_fcontext(transfer.fctx, NULL);
// }

// TEST_CASE("class coroutine") {
//   char stack[STACK_SIZE];
//     void* to_context = make_fcontext(stack + STACK_SIZE, STACK_SIZE, func);
//     transfer_t transfer;
//     printf("I am in main.\n");
//     transfer = jump_fcontext(to_context, NULL);
//     printf("I am in main again!\n");
//     jump_fcontext(transfer.fctx, NULL);
//     printf("End of main\n");
// }