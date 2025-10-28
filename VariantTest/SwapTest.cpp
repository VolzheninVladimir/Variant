#include "pch.h"
#include "Variant.hpp"
#include <string>


TEST(SwapTest, BasicSwapWorks) {
    Variant<int, std::string> v1(std::in_place_type<int>, 10);
    Variant<int, std::string> v2(std::in_place_type<std::string>, "text");

    v1.swap(v2);

    EXPECT_EQ(v1.index(), 1);
    EXPECT_EQ(v2.index(), 0);
    EXPECT_EQ(v1.get<std::string>(), "text");
    EXPECT_EQ(v2.get<int>(), 10);
}

TEST(SwapTest, SameIndexSwapsValues) {
    Variant<int, std::string> v1(std::in_place_type<std::string>, "left");
    Variant<int, std::string> v2(std::in_place_type<std::string>, "right");

    v1.swap(v2);

    EXPECT_EQ(v1.get<std::string>(), "right");
    EXPECT_EQ(v2.get<std::string>(), "left");
}

TEST(SwapTest, BothValuelessDoesNothing) {
    struct ThrowCtor {
        ThrowCtor() { throw std::runtime_error("fail"); }
    };

    Variant<ThrowCtor, int> v1(1);
    Variant<ThrowCtor, int> v2(2);

    try { 
        v1.emplace<ThrowCtor>(); }
    catch (...) {}
    try { v2.emplace<ThrowCtor>(); }
    catch (...) {}

    EXPECT_TRUE(v1.valueless_by_exception());
    EXPECT_TRUE(v2.valueless_by_exception());

    v1.swap(v2);

    EXPECT_TRUE(v1.valueless_by_exception());
    EXPECT_TRUE(v2.valueless_by_exception());
}

TEST(SwapTest, OneValuelessSwapsState) {
    struct ThrowCtor {
        ThrowCtor() { throw std::runtime_error("fail"); }
    };

    Variant<ThrowCtor, std::string> v1(std::string(""));
    Variant<ThrowCtor, std::string> v2(std::in_place_type<std::string>, "alive");

    try { v1.emplace<ThrowCtor>(); }
    catch (...) {}

    EXPECT_TRUE(v1.valueless_by_exception());
    EXPECT_FALSE(v2.valueless_by_exception());

    v1.swap(v2);

    EXPECT_FALSE(v1.valueless_by_exception());
    EXPECT_TRUE(v2.valueless_by_exception());
    EXPECT_EQ(v1.get<std::string>(), "alive");
}

TEST(SwapTest, ExceptionDuringSwapLeavesOneValueless) {
    struct ThrowOnMove {
        ThrowOnMove() = default;
        ThrowOnMove(ThrowOnMove&&) { throw std::runtime_error("move fail"); }
        ThrowOnMove& operator=(ThrowOnMove&&) = default;
    };

    Variant<ThrowOnMove, std::string> v1;

    try { v1 = ThrowOnMove(); }
    catch (...) {}

    Variant<ThrowOnMove, std::string> v2(std::in_place_type<std::string>, "safe");

    try {
        v1.swap(v2);
    }
    catch (...) {
        EXPECT_TRUE(v1.valueless_by_exception() || v2.valueless_by_exception());
    }
}
