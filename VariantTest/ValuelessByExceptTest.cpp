#include "pch.h"
#include "Variant.hpp"


TEST(ValuelessTest, GetThrowsAndBecomesValueless) {
    struct ThrowOnCopy {
        ThrowOnCopy() = default;
        ThrowOnCopy(const ThrowOnCopy&) { throw std::runtime_error("copy fail"); }
    };

    Variant<ThrowOnCopy, int> v(std::in_place_type<int>, 42);
    Variant<ThrowOnCopy, int> broken;

    try {
        broken = v;
    }
    catch (...) {
        EXPECT_TRUE(broken.valueless_by_exception());
    }
}

TEST(ValuelessTest, EmplaceThrowsAndBecomesValueless) {
    struct ThrowCtor {
        ThrowCtor(int) { throw std::runtime_error("ctor fail"); }
    };

    Variant<ThrowCtor, int> v(std::in_place_type<int>, 1);

    try {
        v.emplace<0>(123);
    }
    catch (...) {
        EXPECT_TRUE(v.valueless_by_exception());
    }
}

TEST(ValuelessTest, AssignmentThrowsAndBecomesValueless) {
    struct ThrowAssign {
        ThrowAssign() = default;
        ThrowAssign(const ThrowAssign&) = default;
        ThrowAssign& operator=(const ThrowAssign&) {
            throw std::runtime_error("assign fail");
        }
    };

    Variant<ThrowAssign, int> v1(std::in_place_type<ThrowAssign>);
    Variant<ThrowAssign, int> v2(std::in_place_type<ThrowAssign>);

    try {
        v1 = v2;
    }
    catch (...) {
        EXPECT_TRUE(v1.valueless_by_exception());
    }
}

