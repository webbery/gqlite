#pragma once

struct gast;
class GClause {
public:
  virtual int Parse(struct gast* ast) = 0;
};