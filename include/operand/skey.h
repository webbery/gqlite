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