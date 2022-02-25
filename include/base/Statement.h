#pragma once

struct gast;
class GStatement {
public:
  virtual int Parse(struct gast* ast) = 0;
  virtual int Dump() = 0;
};