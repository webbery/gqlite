#include "base/PACTree.h"
#include "base/Variant.h"
#include <catch.hpp>

class RightValue {
public:
    bool _value{ 0 };
    RightValue() {}
    RightValue(const RightValue& other) {
        this->_value = other._value;
        CHECK(false);
    }
    RightValue(RightValue&& right) noexcept {
        this->_value = right._value;
        right._value = 0;
        CHECK(true);
    }

    RightValue& operator = (const RightValue& other) {
        this->_value = other._value;
        CHECK(false);
        return *this;
    }

    RightValue& operator = (RightValue&& other) noexcept {
        this->_value = other._value;
        other._value = 0;
        CHECK(true);
        return *this;
    }

    ~RightValue() {
        _value = 0;
    }
};

RightValue getRVtest() {
    RightValue rv;
    return rv;
}

TEST_CASE("right value test") {
    RightValue rv;
    rv._value = true;
    RightValue rv2 = getRVtest();
    //rv2 = rv;
}
