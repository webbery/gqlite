#pragma once

/*
 * scan key of b-tree strategy
 */
enum class BTScanStrategy {
  BTLessStrategy = 1,
  BTLessThanStrategy,
  BTEqualStrategy,
  BTGreaterEqualStrategy,
  BTGreaterStrategy,
  BTMaxStrategy
};

#define STR_EQUAL             "eq"
#define STR_LESS_THAN         "lt"
#define STR_LESS_THAN_EQUAL   "lte"
#define STR_GREATE_THAN       "gt"
#define STR_GREATE_THAN_EQUAL "gte"
