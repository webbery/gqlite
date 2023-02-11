#include "base/gvm/Value.h"
#include <fmt/printf.h>

void printValue(const Value& value) {
  value.visit([](double v){
    fmt::print("{}", v);
  },
  [](std::string v) {
    fmt::print("{}", v);
  },
  [](bool v) {
    fmt::print("{}", v);
  });
}

Value operator + (const Value& left, const Value& right) {
  Value value;
  left.visit([&right, &value](double lv) {
    value = right.visit([&lv](double rv) {
      return lv + rv;
    },
    [&lv](std::string rv) ->double {
      throw std::runtime_error("an error accour when number + string");
    },
    [&lv](bool rv) -> double {
      throw std::runtime_error("an error accour when number + boolean");
    });
  },
  [&right, &value](std::string lv) {
    value = right.visit([&lv](double rv) ->std::string {
      throw std::runtime_error("an error accour when string + number");
    },
    [&lv](std::string rv) -> std::string {
      return lv + rv;
    },
    [&lv](bool rv) -> std::string {
      throw std::runtime_error("an error accour when string + boolean");
    },
    [&lv](int rv) -> std::string {
      throw std::runtime_error("an error accour when string + number");
    },
      [&lv](long rv) -> std::string {
      throw std::runtime_error("an error accour when string + number");
    },
    [&lv](uint64_t rv) -> std::string {
      throw std::runtime_error("an error accour when string + number");
    });
  },
  [&right, &value](bool lv) {
    value = right.visit([&lv](double rv) ->bool {
      throw std::runtime_error("an error accour when bool + double");
    },
    [&lv](std::string rv) -> bool {
      throw std::runtime_error("an error accour when bool + string");
    },
    [&lv](bool rv) -> bool {
      return lv + rv;
    }/*,
    [&lv](int rv) -> bool {
      throw std::runtime_error("an error accour when bool + number");
    },
    [&lv](long rv) -> bool {
      throw std::runtime_error("an error accour when bool + number");
    },
    [&lv](uint64_t rv) -> bool {
      throw std::runtime_error("an error accour when bool + number");
    }*/);
  },
  [&right, &value](int lv) {
    value = right.visit([&lv](double rv) ->double {
      return lv + rv;
    },
    [&lv](std::string rv) -> double {
      throw std::runtime_error("an error accour when number + string");
    },
    [&lv](bool rv) -> double {
      return lv + rv;
    });
  },
  [&right, &value](long lv) {
    value = right.visit([&lv](double rv) ->double {
      return lv + rv;
      },
      [&lv](std::string rv) -> double {
        throw std::runtime_error("an error accour when number + string");
      },
      [&lv](bool rv) -> double {
        return lv + rv;
      });
  },
  [&right, &value](uint64_t lv) {
    value = right.visit([&lv](double rv) ->double {
      return lv + rv;
      },
      [&lv](std::string rv) -> double {
        throw std::runtime_error("an error accour when number + string");
      },
      [&lv](bool rv) -> double {
        return lv + rv;
      }/*,
        [&lv](int rv) -> double {
        return lv + rv;
      },
        [&lv](long rv) -> double {
        return lv + rv;
      },
        [&lv](uint64_t rv) -> double {
        return lv + rv;
      }*/);
  });
  return value;
}

Value operator - (const Value& left, const Value& right) {
  Value value;
  left.visit([&right, &value](double lv) {
    value = right.visit([&lv](double rv) {
      return lv - rv;
    },
    [&lv](std::string rv) ->double {
      throw std::runtime_error("an error accour when number - string");
    },
    [&lv](bool rv) -> double {
      throw std::runtime_error("an error accour when number - boolean");
    });
  },
  [&right, &value](std::string lv) {
    value = right.visit([&lv](double rv) ->std::string {
      throw std::runtime_error("an error accour when string - number");
    },
    [&lv](std::string rv) -> std::string {
      throw std::runtime_error("an error accour when string - string");
    },
    [&lv](bool rv) -> std::string {
      throw std::runtime_error("an error accour when string - boolean");
    });
  },
  [&right, &value](bool lv) {
    value = right.visit([&lv](double rv) ->bool {
      throw std::runtime_error("an error accour when boolean - double");
    },
    [&lv](std::string rv) -> bool {
      throw std::runtime_error("an error accour when boolean - string");
    },
    [&lv](bool rv) -> bool {
      return lv - rv;
    });
  });
  return value;
}

Value operator * (const Value& left, const Value& right) {
  Value value;
  left.visit([&right, &value](double lv) {
    value = right.visit([&lv](double rv) {
      return lv * rv;
    },
    [&lv](std::string rv) ->double {
      throw std::runtime_error("an error accour when number * string");
    },
    [&lv](bool rv) -> double {
      throw std::runtime_error("an error accour when number * boolean");
    });
  },
  [&right, &value](std::string lv) {
    value = right.visit([&lv](double rv) ->std::string {
      throw std::runtime_error("an error accour when string * number");
    },
    [&lv](std::string rv) -> std::string {
      throw std::runtime_error("an error accour when string * string");
    },
    [&lv](bool rv) -> std::string {
      throw std::runtime_error("an error accour when string * string");
    });
  },
  [&right, &value](bool lv) {
    value = right.visit([&lv](double rv) ->bool {
      throw std::runtime_error("an error accour when boolean * double");
    },
    [&lv](std::string rv) -> bool {
      throw std::runtime_error("an error accour when boolean * string");
    },
    [&lv](bool rv) -> bool {
      return lv * rv;
    });
  });
  return value;
}

Value operator / (const Value& left, const Value& right) {
  Value value;
  left.visit([&right, &value](double lv) {
    value = right.visit([&lv](double rv) {
      return lv / rv;
    },
    [&lv](std::string rv) ->double {
      throw std::runtime_error("an error accour when number / string");
    },
    [&lv](bool rv) -> double {
      throw std::runtime_error("an error accour when number / boolean");
    });
  },
  [&right, &value](std::string lv) {
    value = right.visit([&lv](double rv) ->std::string {
      throw std::runtime_error("an error accour when string / string");
    },
    [&lv](std::string rv) -> std::string {
      throw std::runtime_error("an error accour when string / string");
    },
    [&lv](bool rv) -> std::string {
      throw std::runtime_error("an error accour when string / boolean");
    });
  }
  ,
  [&right, &value](bool lv) {
    value = right.visit([&lv](double rv) ->bool {
      throw std::runtime_error("an error accour when boolean / double");
    },
    [&lv](std::string rv) -> bool {
      throw std::runtime_error("an error accour when boolean / string");
    },
    [&lv](bool rv) -> bool {
      throw std::runtime_error("an error accour when boolean / boolean");
    });
  });
  return value;
}