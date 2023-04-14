#pragma once
#include "Schedule.h"

class GExecutor;
class GFrogSchedule: public GSchedule {
public:
  GFrogSchedule();

  void step();
  
private:
  void PStep();
  /*
   * hybrid partition
   */
  void SStep();
};
