#include "pch.h"
#include "Variant.hpp"
#include <string>

TEST(AssignmentTest, CopyAssignPreservesValue) {
    Variant<int, std::string> v1(std::in_place_type<std::string>, "copy");
    Variant<int, std::string> v2;
    v2 = v1;
    EXPECT_EQ(v2.index(), 1);
    EXPECT_EQ(v2.get<std::string>(), "copy");
}

TEST(AssignmentTest, CopyAssignThrowsAndBecomesValueless) {
    struct ThrowOnCopy {
        ThrowOnCopy() = default;
        ThrowOnCopy(const ThrowOnCopy&) { throw std::runtime_error("copy fail"); }
    };

    Variant<ThrowOnCopy, int> v1(std::in_place_type<ThrowOnCopy>);
    Variant<ThrowOnCopy, int> v2(std::in_place_type<int>, 42);

    try {
        v2 = v1;
    }
    catch (...) {
        EXPECT_TRUE(v2.valueless_by_exception());
    }
}

TEST(AssignmentTest, MoveAssignPreservesValue) {
    Variant<int, std::string> v1(std::in_place_type<std::string>, "move");
    Variant<int, std::string> v2;
    v2 = std::move(v1);
    EXPECT_EQ(v2.index(), 1);
    EXPECT_EQ(v2.get<std::string>(), "move");
}

TEST(AssignmentTest, MoveAssignThrowsAndBecomesValueless) {
    struct ThrowOnMove {
        ThrowOnMove() = default;
        ThrowOnMove(ThrowOnMove&&) { throw std::runtime_error("move fail"); }
        ThrowOnMove& operator=(ThrowOnMove&&) = default;
    };

    Variant<ThrowOnMove, int> v1(std::in_place_type<ThrowOnMove>);
    Variant<ThrowOnMove, int> v2(std::in_place_type<int>, 7);

    try {
        v2 = std::move(v1);
    }
    catch (...) {
        EXPECT_TRUE(v2.valueless_by_exception());
    }
}

TEST(AssignmentTest, AssignFromValuePreservesType) {
    Variant<int, std::string> v;
    v = std::string("assigned");
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<std::string>(), "assigned");
}

TEST(AssignmentTest, AssignFromValueThrowsAndBecomesValueless) {
    struct ThrowCtor {
        ThrowCtor() = default;
        ThrowCtor(const ThrowCtor&) = delete;
        ThrowCtor(ThrowCtor&&) { throw std::runtime_error("ctor fail"); }
    };

    Variant<ThrowCtor, int> v(std::in_place_type<int>, 99);

    try {
        v = ThrowCtor{};
    }
    catch (...) {
        EXPECT_TRUE(v.valueless_by_exception());
    }
}

