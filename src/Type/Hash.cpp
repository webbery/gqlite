#include "Type/Hash.h"

namespace gql {
#define MIN_LATITUDE    -90.0
#define MAX_LATITUDE    90.0
#define MIN_LONGITUDE   -180.0
#define MAX_LONGITUDE    180.0

#define HASH_PRECISION  6

  char charMap[33] = "0123456789bcdefghjkmnpqrstuvwxyz";
  struct Interval {
    double _low;
    double _high;
  };

  GeoHash::GeoHash(double dLong, double dLat) {
      this->_hash = encode(dLong, dLat);
  }

  GeoHash::GeoHash(const std::string& hash)
  :_hash(hash) { }

  std::string GeoHash::encode(double dLong, double dLat) {
    char hash[HASH_PRECISION + 1] = {0};
    if(dLat <= 90.0 && dLat >= -90.0 && dLong <= 180.0 && dLong >= -180.0) {
      Interval intervalLat = {MIN_LATITUDE, MAX_LATITUDE};
      Interval intervalLong = {MIN_LONGITUDE, MAX_LONGITUDE};
      Interval* pInterval;
      double coord;
      bool isEven = 1;
      unsigned int hashChar = 0;
      for (int idx = 1; idx < HASH_PRECISION; ++idx) {
        if (isEven) {
          pInterval = &intervalLong;
          coord = dLong;
        } else {
          pInterval = &intervalLat;
          coord = dLat;
        }
        double mid = (pInterval->_high + pInterval->_low) / 2;
        hashChar = hashChar << 1;
        if (coord > mid) {
          hashChar |= 1;
          pInterval->_low = mid;
        } else {
          pInterval->_high = mid;
        }

        if (!(idx %5)) { // 2^5对应32个编码
            hash[(idx - 1) / 5] = charMap[hashChar];
            hashChar = 0;
        }
        isEven = !isEven;
      }
    }
    return hash;
  }

  int GeoHash::indexChar(char c, const std::string& str) {
    int cnt = str.size();
        for (int idx = 0; idx < cnt; ++idx) {
            if (c == str[idx]) {
                return idx;
            }
        }
        return -1;
  }

  std::string GeoHash::getNeighbor(const std::string& hash, GeoHash::Direction direction) {
    char *evenNeighbors[] = {"p0r21436x8zb9dcf5h7kjnmqesgutwvy",
                              "bc01fg45238967deuvhjyznpkmstqrwx", 
                              "14365h7k9dcfesgujnmqp0r2twvyx8zb",
                              "238967debc01fg45kmstqrwxuvhjyznp"
                              };
    char *oddNeighbors[] = {"bc01fg45238967deuvhjyznpkmstqrwx", 
                              "p0r21436x8zb9dcf5h7kjnmqesgutwvy",
                              "238967debc01fg45kmstqrwxuvhjyznp",
                              "14365h7k9dcfesgujnmqp0r2twvyx8zb"    
                              };
    char *evenBorders[] = {"prxz", "bcfguvyz", "028b", "0145hjnp"};
    char *oddBorders[] = {"bcfguvyz", "prxz", "0145hjnp", "028b"};
    char lastChar = hash[hash.size() - 1];
    bool isOdd = hash.size() % 2;
    char **border = isOdd ? oddBorders : evenBorders;
    char **neighbor = isOdd ? oddNeighbors : evenNeighbors;
    std::string base;
    base += hash.substr(0, hash.size() - 1);
    if (indexChar(lastChar, border[direction]) != -1) {
        base = getNeighbor(base, direction);
    }
    int neighborIndex = indexChar(lastChar, neighbor[direction]);
    lastChar = charMap[neighborIndex];
    char lastHash[2] = {lastChar, 0};
    base += lastHash;
    return base;
  }

  std::string GeoHash::getNeighbor(GeoHash::Direction direction) {
      return getNeighbor(this->_hash, direction);
  }
}