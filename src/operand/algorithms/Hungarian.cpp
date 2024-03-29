#include "operand/algorithms/Hungarian.h"
#include "gqlite.h"
#include <set>
#include <numeric>
#include <algorithm>
#include <functional>
#include <vector>
#ifdef _PRINT_FORMAT_
#include <iostream>
#include <fmt/printf.h>
#include <fmt/ranges.h>
#include <fmt/format.h>
#endif

#ifdef _PRINT_FORMAT_
template <> struct fmt::formatter<Eigen::MatrixXd> {
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
    return ctx.begin();
  }
  template <typename FormatContext>
  auto format(const Eigen::MatrixXd& p, FormatContext& ctx) const -> decltype(ctx.out()) {
    Eigen::IOFormat fmt(Eigen::FullPrecision, 0, ", ", ";\n", "[", "]", "[", "]");
    std::stringstream ss;
    ss << p.format(fmt);
    return format_to(ctx.out(), "{}", ss.str());
  }
};
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

  bool is_exist(size_t col, const std::set<size_t>& cols, size_t N) {
    if (cols.find(col) != cols.end()) return true;
    return false;
  }

  bool find_new(
    const Eigen::MatrixXd& input,
    size_t row,
    std::set<size_t> usedCols,
    std::list<std::pair<size_t, size_t>>& result)
  {
    // fmt::print("row: {}, new: {}\n", row, usedCols);
    if (row == input.rows() && usedCols.size() == input.cols()) return true;
    for (size_t col = 0; col < input.cols(); ++col) {
      if (usedCols.count(col)) continue;
      if (input(row, col) == 0) {
        auto status = usedCols.insert(col);
        if (find_new(input, row + 1, usedCols, result)) {
          // fmt::print("get it: {}, {}; {}\n", row, col, usedCols);
          result.push_front({row, col});
          return true;
        } else {
          usedCols.erase(status.first);
          // fmt::print("try: {}, {}\n", row, col);
        }
      }
    }
    return false;
  }
  // find valid index
  std::list<std::pair<size_t, size_t>> get_valid_indexes(const Eigen::MatrixXd& input) {
    std::set<size_t> usedCols;
    std::list<std::pair<size_t, size_t>> result;
    find_new(input, 0, usedCols, result);
    return result;
  }
}
int HungorianAlgorithm::solve(const Eigen::MatrixXd& input, std::list<std::pair<size_t, size_t>>& matched) {
  // step 0: weight must positive
  Eigen::MatrixXd m(input);
#ifdef _PRINT_FORMAT_
  fmt::print("prepare:\n{}\n", m);
#endif
  //if (input.rows() > input.cols()) {
  //  auto maxValue = input.maxCoeff() + 1;
  //  Eigen::MatrixXd slice = Eigen::MatrixXd::Constant(m.rows(), m.rows() - m.cols(), maxValue);
  //  m.resize(input.rows(), input.rows());
  //  m << input, slice;
  //}
  //else if (input.cols() > input.rows()) {
  //  auto maxValue = input.maxCoeff() + 1;
  //  Eigen::MatrixXd slice = Eigen::MatrixXd::Constant(m.cols() - m.rows(), m.cols(), maxValue);
  //  m.resize(input.cols(), input.cols());
  //  m << input,
  //       slice;
  //}
#ifdef _PRINT_FORMAT_
  fmt::print("input:\n{}\n", m);
#endif
  const auto minRow = m.rowwise().minCoeff();
  Eigen::MatrixXd m1 = m.colwise() - minRow;
#ifdef _PRINT_FORMAT_
  fmt::print("step 1:\n{}\n", m1);
#endif
  const auto minCol = m1.colwise().minCoeff();
  Eigen::MatrixXd result = m1.rowwise() - minCol;
#ifdef _PRINT_FORMAT_
  fmt::print("step 2:\n{}\n", result);
#endif
  do {
    // get zero count of each row and col, then sort it by count
    size_t total = std::count(result.data(), result.data() + result.size(), 0);
    // fmt::print("step 2:\ntotal {}\n", total);
    size_t times = 0;
    Eigen::MatrixXd mTempInf = result;
    Eigen::MatrixXd mSubMat = result;
    Eigen::MatrixXd mTempLine = Eigen::MatrixXd::Zero(result.rows(), result.cols());
    Eigen::MatrixXd mTempLineCross = Eigen::MatrixXd::Zero(result.rows(), result.cols());
    do {
      size_t ridx = 0, cidx = 0;
      std::vector<size_t> vRowCounts, vColCounts;
      for (size_t row = 0; row < mTempInf.rows(); ++row) {
        const auto& r = mTempInf.row(row);
        size_t c = r.size() - r.count();
        vRowCounts.push_back(c);
      }
      for (size_t col = 0; col < mTempInf.cols(); ++col) {
        const auto& c = mTempInf.col(col);
        size_t value = c.size() - c.count();
        vColCounts.push_back(value);
      }
      // sort
      std::vector<size_t> vRows = sort_indexes(vRowCounts);
      std::vector<size_t> vCols = sort_indexes(vColCounts);
#ifdef _PRINT_FORMAT_
      fmt::print("rows: {}\ncols: {}\n", vRowCounts, vColCounts);
#endif
      size_t rValue = vRowCounts[vRows[ridx]];
      size_t cValue = vColCounts[vCols[cidx]];
      if (rValue > cValue) {
        auto rows = mTempInf.row(vRows[ridx]);
        rows = (rows.array() == 0).select(INFINITY, rows);
        mSubMat.row(vRows[ridx]) = Eigen::VectorXd::Ones(result.cols()) * INFINITY;
        mTempLine.row(vRows[ridx]) = result.row(vRows[ridx]);
        mTempLineCross.row(vRows[ridx]) += Eigen::VectorXd::Ones(result.cols());
        ridx += 1;
      } else {
        auto cols = mTempInf.col(vCols[cidx]);
        cols = (cols.array() == 0).select(INFINITY, cols);
        mSubMat.col(vCols[cidx]) = Eigen::VectorXd::Ones(result.cols()) * INFINITY;
        mTempLine.col(vCols[cidx]) = result.col(vCols[cidx]);
        mTempLineCross.col(vCols[cidx]) += Eigen::VectorXd::Ones(result.cols());
        cidx += 1;
      }
      times += 1;
#ifdef _PRINT_FORMAT_
    fmt::print("replaced: {}\n{}\nmSubMat:{}\n", mTempInf, mTempInf.size(), mSubMat);
#endif
    } while (mTempInf.count() < mTempInf.size());
    if (times >= input.rows()) {
#ifdef _PRINT_FORMAT_
      fmt::print("out:\n{}\n{}\n", result, mTempInf);
#endif
      break;
    }
    double minimal = mSubMat.minCoeff();
    double maximal = mTempLineCross.maxCoeff();
#ifdef _PRINT_FORMAT_
  // fmt::print("{}", minial);
  // fmt::print("step 2:\nsort rows {}, cols {}\n", vRows, vCols);
  step4:
    fmt::print("before:\n{}\n", mTempInf);
#endif
    mSubMat -= Eigen::MatrixXd::Ones(mSubMat.rows(), mSubMat.cols()) * minimal;
    // Eigen::MatrixXd mTemp = (mSubMat.array() != INFINITY).select(0, mTempInf);
    mSubMat = (mSubMat.array() == INFINITY).select(0, mSubMat);
    //mTempInf = mTemp + mSubMat;
#ifdef _PRINT_FORMAT_
  fmt::print("after:\n{}\nsub: {}\n", mTempInf, mSubMat);
#endif
    auto pluss = (mTempLineCross.array() == maximal).select(minimal, Eigen::MatrixXd::Zero(result.rows(), result.cols()));
#ifdef _PRINT_FORMAT_
  fmt::print("line:\n{}\n", mTempLine);
  std::cout<< pluss<<std::endl;
#endif
    result = mSubMat + mTempLine + pluss;
#ifdef _PRINT_FORMAT_
  fmt::print("result:\n{}\n", result);
#endif
  } while (true);
  // get position of matched
#ifdef _PRINT_FORMAT_
  fmt::print("before result:\n{}\n", result);
#endif
  auto indexes = get_valid_indexes(result);
  auto itr = indexes.begin();
  for (size_t offset = 0; offset < input.rows(); ++itr, ++offset);
  matched.assign(indexes.begin(), itr);
#ifdef _PRINT_FORMAT_
  fmt::print("result:\n{}\n", matched);
#endif
  return ECode_Success;
}

int HungorianAlgorithm::solve(const Eigen::MatrixXd& m, double& weight) {
  std::list<std::pair<size_t, size_t>> indexes;
  int ret = this->solve(m, indexes);
  if (ret == ECode_Success) {
    weight = 0;
    for (auto indx = indexes.begin(); indx != indexes.end(); ++indx) {
      weight += m(indx->first, indx->second);
    }
  }
  return ret;
}