#pragma once
#include "json.hpp"
#include "mdbx.h++"
#include "Type/Type.h"
#include "Error.h"
#include <set>

enum class FeatureType {
  GTUndefined,
  GTIntegerFeature,
  GTStringFeature
};

template<typename T> struct slice_traits {
  static size_t size(T) { return sizeof(T); }
};

template<typename T> struct feature_traits { static const FeatureType type = FeatureType::GTStringFeature; };
template<> struct feature_traits<uint64_t> {
  static const FeatureType type = FeatureType::GTIntegerFeature;
};
template<> struct feature_traits<std::string> {
  static const FeatureType type = FeatureType::GTStringFeature;
};

template<typename T>
int move_cursor(mdbx::txn_managed& txn, mdbx::map_handle& handle, T pos, mdbx::cursor_managed& cursor) {
  using namespace mdbx;
  cursor = txn.open_cursor(handle);
  if (cursor.on_last()) return ECode_GQL_Vertex_Not_Exist;
  mdbx::slice key(&pos, slice_traits< T >::size(pos));
  cursor.lower_bound(key);
  return ECode_Success;
}

class GGraph;
/**
 * @brief vertex property feature is apply when this property of vertex is upset
 */
class GVertexProptertyFeature {
public:
  GVertexProptertyFeature(const char* property);

  virtual int apply(mdbx::txn_managed& txn, const std::string& id, const std::string& key, const nlohmann::json& value) = 0;
  
  virtual ~GVertexProptertyFeature();

  int get_cursor(mdbx::txn_managed& txn, const std::any& pos, NodeType nt, mdbx::cursor_managed& cursor) {
    switch (nt)
    {
    case NodeType::Number:
      // index must be integer not double
      anchor_cursor(txn, std::any_cast<uint64_t>(pos), cursor);
      break;
    case NodeType::String:
      break;
    default:
      break;
    }
    return ECode_Success;
  }

  std::string indexName() {return _property;}
  std::string name() {return _idname;}
  bool drop(mdbx::txn_managed& txn);

protected:
  mdbx::map_handle& create(mdbx::txn_managed& txn, FeatureType type);

private:
  template<typename T>
  int anchor_cursor(mdbx::txn_managed& txn, T pos, mdbx::cursor_managed& cursor) {
    _handle = create(txn, feature_traits< T >::type);
    using namespace mdbx;
    return move_cursor(txn, _handle, pos, cursor);
  }

protected:
  std::string _property;
  std::string _idname;
  FeatureType _type;
  mdbx::map_handle _handle;
};

/**
 * @brief this kind of feature is to extend new kind of data.
 */
class GDatumTypeFeature {
public:
  /**
   * @brief check input belong this type or not 
   */
  virtual bool match() = 0;
};