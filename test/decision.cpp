#include "Graph/DecisionNetwork.h"
#include "operand/algorithms/DecisionTree.h"
#include <catch.hpp>

TEST_CASE("decision tree") {
  GDecisionTree* tree = new GDecisionTree();
  tree->addNode("index");
  tree->addNode("node_cnt");
  tree->addNode("node>index", "node>index");
  tree->addNode("node<=index", "node<=index");
  tree->addNode("index_not_exist", "index_not_exist");

  tree->addEdge("index", "index_not_exist", range<true, false>{-INFINITY, 0});
  tree->addEdge("index", "node_cnt", range<true, true>{0, INFINITY});
  tree->addEdge("node_cnt", "node>index", range<true, false>{-INFINITY, 0});
  tree->addEdge("node_cnt", "node<index", range<true, true>{0, INFINITY});

  delete tree;
}

TEST_CASE("decision network, case 1") {
  /*
    P(A = +, B = +, C = +)
    P(B = -, C = + | A = +)
    P(A = - | C = +)
    ******
    A
    0.4
    ***
    B | A
    0.6 +
    0.3 -
    ***
    C | B
    0.7 +
    0.2 -
  */
  GDecisionNetwork* dn = new GDecisionNetwork();
  delete dn;
}

TEST_CASE("decision network, case 2") {
  /*
    P(D = + | L = -, I = +)
    EU(I = +)
    EU(I = + | L = +)
    MEU(I)
    MEU(I | L = +)
    ******
    L
    0.4
    ***
    N | L
    0.8 +
    0.3 -
    ***
    I
    decision
    ***
    D | N I
    0.3 + +
    0.6 + -
    0.95 - +
    0.05 - -
    ******
    utility | D
    100 +
    -10 -
  */
  GDecisionNetwork* dn = new GDecisionNetwork();
  delete dn;
}