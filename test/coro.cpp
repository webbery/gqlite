#include "base/system/Coroutine.h"

int main(int argc, char* argv[]) {
  GCoSchedule schedule;
  schedule.createCoroutine([](GCoroutine* co) {printf("1\n"); });
  schedule.createCoroutine([](GCoroutine* co) {printf("2\n"); });
  schedule.run();
  schedule.join();
}