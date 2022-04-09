#include "operand/algorithms/Hungarian.h"
#include "gqlite.h"
#include <numeric>
#include "Graph/BipartiteGraph.h"
#include <algorithm>
#ifdef _PRINT_FORMAT_
#include <iostream>
#include <fmt/printf.h>
#include <fmt/ranges.h>
#endif

namespace {
  template< typename T >
  std::vector<size_t> sort_indexes(const std::vector<T>& v) {
    std::vector<size_t> indx(v.size());
    iota(indx.begin(), indx.end(), 0);
    stable_sort(indx.begin(), indx.end(), [&v](size_t ia, size_t ib) {
      return v[ia] > v[ib];
    });
    return indx;
  }
}
int HungorianAlgorithm::solve(Eigen::MatrixXd& m, Eigen::MatrixXd& out) {
  const auto minRow = m.rowwise().minCoeff();
  // std::cout<< minMat<<std::endl;
  Eigen::MatrixXd m1 = m.colwise() - minRow;
#ifdef _PRINT_FORMAT_
  fmt::print("step 1:\n{}\n", m1);
#endif
  const auto minCol = m1.colwise().minCoeff();
  Eigen::MatrixXd m2 = m1.rowwise() - minCol;
#ifdef _PRINT_FORMAT_
  fmt::print("step 2:\n{}\n", m2);
#endif
  // get zero count of each row and col, then sort it by count
  size_t total = std::count(m2.data(), m2.data() + m2.size(), 0);
  // fmt::print("step 2:\ntotal {}\n", total);
  std::vector<size_t> vRowCounts, vColCounts;
  for (size_t row = 0; row<m2.rows(); ++row) {
    const auto& r = m2.row(row);
    size_t c = r.size() - r.count();
    vRowCounts.push_back(c);
  }
  for (size_t col = 0; col < m2.cols(); ++col) {
    const auto& c = m2.col(col);
    size_t value = c.size() - c.count();
    vColCounts.push_back(value);
  }
  // sort
  std::vector<size_t> vRows = sort_indexes(vRowCounts);
  std::vector<size_t> vCols = sort_indexes(vColCounts);
  size_t ridx = 0, cidx = 0;
  Eigen::MatrixXd mTempInf = m2;
  Eigen::MatrixXd mTempLine = Eigen::MatrixXd::Zero(m2.rows(), m2.cols());
  Eigen::MatrixXd mTempLineCross = Eigen::MatrixXd::Zero(m2.rows(), m2.cols());
  do {
    size_t rValue = vRowCounts[vRows[ridx]];
    size_t cValue = vColCounts[vCols[cidx]];
    if (rValue > cValue) {
      mTempInf.row(vRows[ridx]).fill(INFINITY);
      mTempLine.row(vRows[ridx]) = m2.row(vRows[ridx]);
      mTempLineCross.row(vRows[ridx]) += Eigen::VectorXd::Ones(m2.cols());
      ridx += 1;
    } else if (cValue > rValue) {
      mTempInf.col(vCols[cidx]).fill(INFINITY);
      mTempLine.col(vCols[cidx]) = m2.col(vCols[cidx]);
      mTempLineCross.col(vCols[cidx]) += Eigen::VectorXd::Ones(m2.cols());
      cidx += 1;
    }
    // fmt::print("replaced: {}, {}\n", mTemp, mTemp.size());
  } while (mTempInf.count() < mTempInf.size());
  double minimal = mTempInf.minCoeff();
  double maximal = mTempLineCross.maxCoeff();
#ifdef _PRINT_FORMAT_
  // fmt::print("{}", minial);
  // fmt::print("step 2:\nsort rows {}, cols {}\n", vRows, vCols);
  step4:
  fmt::print("before:\n{}\n", mTempInf);
#endif
  mTempInf -= Eigen::MatrixXd::Ones(mTempInf.rows(), mTempInf.cols()) * minimal;
  mTempInf = (mTempInf.array() == INFINITY).select(0, mTempInf);
#ifdef _PRINT_FORMAT_
  fmt::print("after:\n{}\n", mTempInf);
#endif
  auto pluss = (mTempLineCross.array() == maximal).select(minimal, Eigen::MatrixXd::Zero(m2.rows(), m2.cols()));
#ifdef _PRINT_FORMAT_
  fmt::print("line:\n{}\n", mTempLine);
  std::cout<< pluss<<std::endl;
#endif
  out = mTempInf + mTempLine + pluss;
#ifdef _PRINT_FORMAT_
  fmt::print("out:\n{}\n", out);
#endif
  return ECode_Success;
}