#include "base/gvm/Value.h"
#include <fmt/printf.h>

void printValue(const Value& value) {
  value.visit([](double v){
    fmt::printf("{}", v);
  },
  [](std::string v) {
    fmt::printf("{}", v);
  },
  [](bool v) {
    fmt::printf("{}", v);
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
    });
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