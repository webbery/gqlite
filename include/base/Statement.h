#pragma once

struct gast;
class GStatement {
public:
  virtual ~GStatement() {}
  virtual int Parse(struct gast* ast) = 0;
  virtual int Dump() = 0;
};