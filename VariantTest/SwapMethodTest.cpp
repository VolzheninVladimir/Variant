#include "pch.h"
#include "Variant.hpp"
#include <string>

namespace {
    struct NoOperator {
        NoOperator() = default;
        NoOperator(int) = delete;
        NoOperator(const NoOperator&) = delete;
        NoOperator(NoOperator&&) = delete;

        NoOperator& operator=(const NoOperator&) = delete;
        NoOperator& operator=(NoOperator&&) = delete;
        bool operator==(const NoOperator&) const = delete;
    };

    struct ThrowingType {
        ThrowingType() = default;
        ThrowingType(int) {
            throw std::runtime_error("construct fail");
        }
        ThrowingType(const ThrowingType&) {
            throw std::runtime_error("copy fail");
        }
        ThrowingType(ThrowingType&&) {
            
        }

        ThrowingType& operator=(const ThrowingType&) {
            throw std::runtime_error("assign copy fail");
            return *this;
        }
        ThrowingType& operator=(ThrowingType&&) {

            return *this;
        }
        bool operator==(const ThrowingType&) const {
            return true;
        }
    };

    struct Tracker {
        static inline bool copied = false;
        static inline bool assignCopied = false;
        static inline bool assignMoved = false;
        static inline bool moved = false;
        static inline bool destroyed = false;
        static inline bool compared = false;

        int value = 0;

        Tracker() = default;
        Tracker(int v) : value(v) {}
        Tracker(const Tracker&) { copied = true; }
        Tracker(Tracker&&) noexcept { moved = true; }
        ~Tracker() { destroyed = true; }

        Tracker& operator=(const Tracker& other) {
            assignCopied = true;
            return *this;
        }
        Tracker& operator=(Tracker&& other) {
            assignMoved = true;
            return *this;
        }
        bool operator==(const Tracker& other) const {
            compared = true;
            return value == other.value;
        }

        static void reset() {
            copied = false;
            moved = false;
            assignCopied = false;
            assignMoved = false;
            destroyed = false;
            compared = false;
        }
    };
}


TEST(SwapTest, IsNoexceptIf_AllTypesAreNoexceptSwappable) {
    using V = Variant<int, double>;
    if constexpr (noexcept(std::declval<V&>().swap(std::declval<V&>()))) {
        EXPECT_TRUE(true);
    }
    else {
        EXPECT_TRUE(false);
    }
}

TEST(SwapTest, IsNotNoexceptIf_AnyTypeIsThrowingSwappable) {
    using V = Variant<ThrowingType>;
    if constexpr (!noexcept(std::declval<V&>().swap(std::declval<V&>()))) {
        EXPECT_TRUE(true);
    }
    else {
        EXPECT_TRUE(false);
    }
}

TEST(SwapTest, DoesNothingIf_BothAreValueless) {
    Variant<ThrowingType> v1, v2;
    try { v1.emplace<ThrowingType>(1); }
    catch (...) {}
    try { v2.emplace<ThrowingType>(2); }
    catch (...) {}
    v1.swap(v2);
    EXPECT_TRUE(v1.valueless_by_exception());
    EXPECT_TRUE(v2.valueless_by_exception());
}

TEST(SwapTest, SwapsValuesIf_SameTypeAndSameIndex) {
    Variant<int> v1(123), v2(456);
    v1.swap(v2);
    EXPECT_EQ(v1.get<int>(), 456);
    EXPECT_EQ(v2.get<int>(), 123);
}

TEST(SwapTest, MovesIf_OneIsValueless) {
    Variant<ThrowingType> v1, v2;
    try { v2.emplace<ThrowingType>(1); }
    catch (...) {}
    try {
        v1.swap(v2);
    }
    catch (...) {}
    EXPECT_TRUE(v1.valueless_by_exception());
    EXPECT_FALSE(v2.valueless_by_exception());
}

TEST(SwapTest, SwapsDifferentTypesCorrectly) {
    Variant<int, double> v1(42), v2(3.14);
    v1.swap(v2);
    EXPECT_EQ(v1.index(), 1);
    EXPECT_EQ(v1.get<double>(), 3.14);
    EXPECT_EQ(v2.index(), 0);
    EXPECT_EQ(v2.get<int>(), 42);
}

TEST(SwapTest, InvokesStdSwap_IfSameType) {
    Tracker::reset();
    Variant<Tracker> v1(std::in_place_index<0>, 1);
    Variant<Tracker> v2(std::in_place_index<0>, 2);
    v1.swap(v2);
    EXPECT_TRUE(Tracker::assignMoved);
    EXPECT_TRUE(Tracker::moved);
    EXPECT_TRUE(Tracker::destroyed);
}



