#include "pch.h"
#include "Variant.hpp"
#include <string>

TEST(EmplaceTest, EmplaceByTypeConstructsCorrectly) {
    Variant<int, std::string> v;
    v.emplace<std::string>("hello");
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<std::string>(), "hello");
}

TEST(EmplaceTest, EmplaceByTypeThrowsAndBecomesValueless) {
    struct ThrowCtor {
        ThrowCtor(int) { throw std::runtime_error("fail"); }
    };

    Variant<ThrowCtor, int> v(std::in_place_type<int>, 42);
    try {
        v.emplace<ThrowCtor>(123);
    }
    catch (...) {
        EXPECT_TRUE(v.valueless_by_exception());
    }
}

TEST(EmplaceTest, EmplaceByTypeInitListConstructsCorrectly) {
    Variant<std::vector<int>, std::string> v;
    v.emplace<std::vector<int>>({ 1, 2, 3 });
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<std::vector<int>>().size(), 3);
}

TEST(EmplaceTest, EmplaceByTypeInitListThrowsAndBecomesValueless) {
    struct ThrowInit {
        ThrowInit(std::initializer_list<int>) { throw std::runtime_error("fail"); }
    };

    Variant<ThrowInit, int> v(std::in_place_type<int>, 7);
    try {
        v.emplace<ThrowInit>({ 1, 2, 3 });
    }
    catch (...) {
        EXPECT_TRUE(v.valueless_by_exception());
    }
}

TEST(EmplaceTest, EmplaceByIndexConstructsCorrectly) {
    Variant<int, std::string> v;
    v.emplace<1>("indexed");
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<1>(), "indexed");
}

TEST(EmplaceTest, EmplaceByIndexThrowsAndBecomesValueless) {
    struct ThrowCtor {
        ThrowCtor(int) { throw std::runtime_error("fail"); }
    };

    Variant<ThrowCtor, int> v(std::in_place_type<int>, 99);
    try {
        v.emplace<0>(123);
    }
    catch (...) {
        EXPECT_TRUE(v.valueless_by_exception());
    }
}

TEST(EmplaceTest, EmplaceByIndexInitListConstructsCorrectly) {
    Variant<std::vector<int>, std::string> v;
    v.emplace<0>({ 4, 5, 6 });
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<0>().size(), 3);
}

TEST(EmplaceTest, EmplaceByIndexInitListThrowsAndBecomesValueless) {
    struct ThrowInit {
        ThrowInit(std::initializer_list<int>) { throw std::runtime_error("fail"); }
    };

    Variant<ThrowInit, int> v(std::in_place_type<int>, 1);
    try {
        v.emplace<0>({ 1, 2 });
    }
    catch (...) {
        EXPECT_TRUE(v.valueless_by_exception());
    }
}
