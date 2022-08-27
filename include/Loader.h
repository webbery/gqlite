#pragma once

class GScanPlan;
class GLoader {
public:
  GLoader(GScanPlan* plan);

  bool load();
private:
};