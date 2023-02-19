#include "base/gvm/Value.h"
#include <fmt/printf.h>
#include "base/gvm/Object.h"

void printValue(const Value& value) {
  value.visit([](double v){
    fmt::print("{}", v);
  },
  [](std::string v) {
    fmt::print("{}", v);
  },
  [](bool v) {
    fmt::print("{}", v);
  },
  [](int v) {
    fmt::print("{}", v);
  },
  [](long v) {
    fmt::print("{}", v);
  },
  [](uint64_t v) {
    fmt::print("{}", v);
  },
  [](FunctionObj* v) {
    fmt::print("function <{}>", v->name);
  },
  [](NativeObj* v) {
    fmt::print("native function");
  });
}

std::string getValueString(const Value& value) {
  std::string s;
  value.visit([&s](double v){
    s += std::to_string(v);
  },
  [&s](std::string v) {
    s += v;
  },
  [&s](bool v) {
    if (v) {
      s += "true";
    } else {
      s += "false";
    }
  },
  [&s](int v) {
    s += std::to_string(v);
  },
  [&s](long v) {
    s += std::to_string(v);
  },
  [&s](uint64_t v) {
    s += std::to_string(v);
  });
  return s;
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
    [&lv](int rv) {
      return lv + rv;
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
  [&right, &value](int lv) {
    value = right.visit([&lv](double rv) ->double {
      return lv + rv;
      },
      [&lv](int rv) ->double {
        return lv + rv;
      },
      [&lv](long rv) ->double {
        return lv + rv;
      }
    );
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
    [&lv](int rv) {
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
  [&right, &value](int lv) {
    value = right.visit([&lv](double rv) -> double {
      return lv - rv;
      },
      [&lv](int rv) -> double {
        return lv - rv;
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
  }
    );
  return value;
}

Value operator * (const Value& left, const Value& right) {
  Value value;
  left.visit([&right, &value](double lv) {
    value = right.visit([&lv](double rv) {
      return lv * rv;
    },
    [&lv](int rv) {
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
  [&right, &value](int lv) {
    value = right.visit([&lv](double rv) -> double {
      return (double)lv * rv;
      },
      [&lv](int rv) -> double {
        return (double)lv * rv;
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
      if (rv == 0) {
        throw std::runtime_error("divider is 0");
      }
      return lv / rv;
    },
    [&lv](std::string rv) ->double {
      throw std::runtime_error("an error accour when number / string");
    },
    [&lv](int rv) {
      if (rv == 0) {
        throw std::runtime_error("divider is 0");
      }
      return lv / rv;
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
  },
  [&right, &value](int lv) {
    value = right.visit([&lv](double rv) -> double {
      if (rv == 0) {
        throw std::runtime_error("divider is 0");
      }
      return (double)lv / rv;
      },
      [&lv](int rv) -> double {
        if (rv == 0) {
          throw std::runtime_error("divider is 0");
        }
        return (double)lv / rv;
      });
  },
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