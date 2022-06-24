#include <iostream>
#include "base/PACTree.h"

int main() {
  gql::GPACTree<int, int> ptree(2);
  ptree.set(1, 1);
  for (auto itr = ptree.begin(); itr != ptree.end(); ++itr) {
    int value = *itr;
    std::cout<<value<<std::endl;
  }
}