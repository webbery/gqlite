#include "base/system/Coroutine.h"

int main(int argc, char* argv[]) {
  GCoSchedule schedule;
  schedule.createCoroutine([](GCoroutine* co) {printf("1\n"); co->yield(); printf("3\n"); });
  schedule.createCoroutine([](GCoroutine* co) {printf("2\n"); co->yield(); printf("4\n"); });
  schedule.run();
  schedule.join();
}