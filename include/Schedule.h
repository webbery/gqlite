#pragma once

class GSchedule {
public:
  virtual ~GSchedule() {}

  virtual std::future<int> schedule() = 0;
};