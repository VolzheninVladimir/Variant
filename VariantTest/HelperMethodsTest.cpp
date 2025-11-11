#include "pch.h"
#include "Variant.hpp"
#include <string>

namespace {
    struct ThrowingType {
        ThrowingType() = default;
        ThrowingType(int) {
            throw std::runtime_error("construct fail");
        }
        ThrowingType(const ThrowingType&) {
            throw std::runtime_error("copy fail");
        }
        ThrowingType(ThrowingType&&) {
            throw std::runtime_error("move fail");
        }

        ThrowingType& operator=(const ThrowingType&) {
            throw std::runtime_error("assign copy fail");
            return *this;
        }
        ThrowingType& operator=(ThrowingType&&) {
            throw std::runtime_error("assign move fail");
            return *this;
        }
        bool operator==(const ThrowingType&) const {
            return true;
        }
    };
}

TEST(HelperMethodsTest, Index_ReturnsCurrentIndex) {
    Variant<int, double> v(3.14);
    EXPECT_EQ(v.index(), 1);
}

TEST(HelperMethodsTest, ValuelessByException_ReturnsTrueIfValueless) {
    Variant<ThrowingType> v;
    try {
        v.emplace<ThrowingType>(1);
    }
    catch (...) {}
    EXPECT_TRUE(v.valueless_by_exception());
}

TEST(HelperMethodsTest, ValuelessByException_ReturnsFalseIfValid) {
    Variant<int> v(42);
    EXPECT_FALSE(v.valueless_by_exception());
}

TEST(HelperMethodsTest, Npos_IsEqualToMinusOne) {
    constexpr std::size_t expected = static_cast<std::size_t>(-1);
    EXPECT_EQ((Variant<int>::npos), expected);
}

TEST(HelperMethodsTest, HoldsAlternative_ReturnsTrueIfTypeMatches) {
    Variant<int, double> v(3.14);
    EXPECT_TRUE(v.holds_alternative<double>());
    EXPECT_FALSE(v.holds_alternative<int>());
}

TEST(HelperMethodsTest, HoldsAlternative_ReturnsFalseIfValueless) {
    Variant<ThrowingType> v;
    try {
        v.emplace<ThrowingType>(1);
    }
    catch (...) {}
    EXPECT_FALSE(v.holds_alternative<ThrowingType>());
}

