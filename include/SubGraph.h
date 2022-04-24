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
#include <atomic>
#include <memory>
#define COLORED_RED 1

class GSubGraph : public GGraphInterface {
public:
  GSubGraph();
  GSubGraph(const GSubGraph&);
  GSubGraph(GSubGraph&&);
  GSubGraph& operator = (const GSubGraph&);
  GSubGraph& operator = (GSubGraph&&);

  virtual ~GSubGraph();

  virtual int addVertex(const std::string& id, const nlohmann::json& props = nlohmann::json());
  virtual int addEdge(const std::string& from, const std::string& to, GraphEdgeDirection direction, const nlohmann::json& props = nlohmann::json());
  virtual int eraseVertex(const std::string& id);
  virtual int earseEdge(const edge_id& id);

  bool operator == (const GSubGraph& other);

  bool isBipartite();

  typedef std::map<std::string, double> VectorSd;
  GVertex* operator[] (const std::string& vertex);

  enum MatrixType{
    Concat,
    Extend,     // Approximate graph edit distance computation by means of bipartite graph matching. Definition 4
  };
  /**
   * @params weight Specify which property to weight. If null, default weight is 1.
   **/
  Eigen::MatrixXd toMatrix(const char* weight=nullptr);
  /**
   * @brief genereate a bipartite matrix with `from` and `to` vertexes 
   **/
  Eigen::MatrixXd toMatrix(const GSubGraph& from, const GSubGraph& to, MatrixType type = Extend);

  typedef std::map<std::string, GVertex*>::const_iterator vertex_iterator;
  vertex_iterator vertex_begin() const { return _vertexes.begin();}
  vertex_iterator vertex_end()const { return _vertexes.end();}

  typedef std::map<edge_id, GEdge*>::const_iterator edge_iterator;
  edge_iterator edge_begin() const { return _edges.begin(); }
  edge_iterator edge_end() const { return _edges.end(); }

protected:
  uint32_t Ref(uint32_t cnt = 1) {
    _cnt.fetch_add(cnt);
    return _cnt;
  }
  void DeRef(uint32_t cnt = 1) {
    if (_cnt.fetch_sub(cnt) == _cnt) {
#ifdef _PRINT_FORMAT_
      fmt::print("clean graph\n");
#endif
      clean();
    }
  }
  void clean() {
    for (auto itr = _vertexes.begin(), end = _vertexes.end(); itr != end; ++itr) {
      delete itr->second;
    }
    for (auto itr = _edges.begin(), end = _edges.end(); itr != end; ++itr) {
      delete itr->second;
    }
  }

protected:
  std::map<std::string, GVertex*> _vertexes;
  std::map<edge_id, GEdge*> _edges;
private:
  std::atomic_uint32_t _cnt;
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
