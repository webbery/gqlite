#pragma once
#include <string>

namespace gql {
  class GeoHash {
  public:
    GeoHash(double dLong, double dLat);
    GeoHash(const std::string& hash);

    enum Direction {
        North, East, South, West
    };

    std::string getNeighbor(Direction direction);
    std::string toString();

  private:
    std::string encode(double dLong, double dLat);
    std::string getNeighbor(const std::string& hash, Direction direction);
    int indexChar(char c, const std::string& str);
    
  private:
    std::string _hash;
  };
}