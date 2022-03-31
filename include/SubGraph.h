#pragma once
#include <vector>
#include "compiler.h"
#include "Type/Vertex.h"
#include "Type/Edge.h"
#include "IGraph.h"
#include <map>
#include <Eigen/Core>
#ifdef _PRINT_FORMAT_
#include <fmt/format.h>
#include <strstream>
#endif

class GSubGraph : public GGraphInterface {
public:
  virtual ~GSubGraph();

  virtual int addVertex(const std::string& id, const nlohmann::json& props = nlohmann::json());
  virtual int addEdge(const std::string& from, const std::string& to, GraphEdgeDirection direction, const nlohmann::json& props = nlohmann::json());
  virtual int eraseVertex(const std::string& id);
  virtual int earseEdge(const edge_id& id);

  bool operator == (const GSubGraph& other);

  bool isBipartite();

  typedef std::map<std::string, double> VectorSd;
  VectorSd operator[] (const std::string& vertex) const;

  /**
   * @params weight Specify which property to weight. If null, default weight is 1.
   **/
  Eigen::MatrixXd toMatrix(const char* weight=nullptr);
private:
  std::map<std::string, GVertex*> _vertexes;
  std::map<edge_id, GEdge*> _edges;
};

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
