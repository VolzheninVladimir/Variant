#include "pch.h"
#include "Variant.hpp"

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




TEST(OperatorsTest_Equality, FailsIf_TypeDoesNotSupportEqualityOperator) {
    Variant<NoOperator> v1, v2;
    // bool result = (v1 == v2); // static_assert провалится
    EXPECT_TRUE(true);
}

TEST(OperatorsTest_Equality, IsNoexceptIf_AllTypesAreNoexcept) {
    using V = Variant<int, double>;
    EXPECT_TRUE(noexcept(std::declval<V&>() == std::declval<const V&>()));

}

TEST(OperatorsTest_Equality, IsNotNoexceptIf_AnyTypeIsThrowing) {
    using V = Variant<ThrowingType>;
    EXPECT_TRUE(!noexcept(std::declval<V&>() == std::declval<const V&>()));
}

TEST(OperatorsTest_Equality, ReturnsTrueIf_BothAreValueless) {
    Variant<ThrowingType> v1, v2;
    try { v1.emplace<ThrowingType>(1); }
    catch (...) {}
    try { v2.emplace<ThrowingType>(2); }
    catch (...) {}
    EXPECT_TRUE(v1 == v2);
}

TEST(OperatorsTest_Equality, ReturnsFalseIf_OnlyOneIsValueless) {
    Variant<ThrowingType> v1, v2;
    try { v1.emplace<ThrowingType>(1); }
    catch (...) {}
    EXPECT_FALSE(v1 == v2);
    EXPECT_FALSE(v2 == v1);
}

TEST(OperatorsTest_Equality, ReturnsFalseIf_IndicesAreDifferent) {
    Variant<int, double> v1(42), v2(3.14);
    EXPECT_FALSE(v1 == v2);
}

TEST(OperatorsTest_Equality, ReturnsTrueIf_SameTypeAndEqual) {
    Variant<int> v1(123), v2(123);
    EXPECT_TRUE(v1 == v2);
}

TEST(OperatorsTest_Equality, ReturnsFalseIf_SameTypeAndNotEqual) {
    Variant<int> v1(123), v2(456);
    EXPECT_FALSE(v1 == v2);
}

TEST(OperatorsTest_Equality, InvokesEqualityOperatorOf_StoredType) {
    Tracker::reset();
    Variant<Tracker> v1, v2;
    EXPECT_TRUE(v1 == v2);
    EXPECT_TRUE(Tracker::compared);
}


TEST(OperatorsTest_CopyAssignment, FailsIf_TypeIsNotCopyConstructible) {
    Variant<NoOperator> v1, v2;
    // v1 = v2;
    EXPECT_TRUE(true);
}

TEST(OperatorsTest_CopyAssignment, IsNoexceptIf_AllTypesAreNoexceptCopyable) {
    using V = Variant<int, double>;
    EXPECT_TRUE(noexcept(std::declval<V&>() = std::declval<const V&>()));
}

TEST(OperatorsTest_CopyAssignment, IsNotNoexceptIf_AnyTypeIsThrowingCopyable) {
    using V = Variant<ThrowingType>;
    EXPECT_TRUE(!noexcept(std::declval<V&>() = std::declval<const V&>()));
}

TEST(OperatorsTest_CopyAssignment, SelfAssignment_DoesNothing) {
    Variant<int> v(42);
    Variant<int>& ref = v;
    v = ref;
    EXPECT_EQ(v.get<int>(), 42);
}

TEST(OperatorsTest_CopyAssignment, ValidCopy_CopiesCorrectly) {
    Variant<int> v1(123), v2;
    v2 = v1;
    EXPECT_EQ(v2.index(), 0);
    EXPECT_EQ(v2.get<int>(), 123);
}

TEST(OperatorsTest_CopyAssignment, ThrowsAndBecomesValuelessIf_CopyFails) {
    Variant<ThrowingType> v1, v2;
    try {
        v2 = v1;
    }
    catch (...) {
        EXPECT_TRUE(v2.valueless_by_exception());
    }
}

TEST(OperatorsTest_CopyAssignment, DestroysPreviousTypeAnd_CopiConstruct) {
    Tracker::reset();
    Variant<Tracker, int> v1, v2(5);
    v1 = v2;
    EXPECT_TRUE(Tracker::destroyed);
    Variant<Tracker, int> v3;
    v1 = v3;
    EXPECT_TRUE(Tracker::copied);
    EXPECT_EQ(v1.index(), 0);
}

TEST(OperatorsTest_CopyAssignment, DestroysPreviousTypeAnd_CopiAssignment) {
    Tracker::reset();
    Variant<Tracker, int> v1, v2(std::in_place_index<0>, 1);
    v1 = v2;
    EXPECT_FALSE(Tracker::destroyed);
    EXPECT_FALSE(Tracker::copied);
    EXPECT_TRUE(Tracker::assignCopied);
    EXPECT_EQ(v1.index(), 0);
}


TEST(OperatorsTest_MoveAssignment, FailsIf_TypeIsNotMoveConstructible) {
    Variant<NoOperator> v1, v2;
    // v2 = std::move(v1);
    EXPECT_TRUE(true);
}

TEST(OperatorsTest_MoveAssignment, IsNoexceptIf_AllTypesAreNoexceptMovable) {
    using V = Variant<int, double>;
    EXPECT_TRUE(noexcept(std::declval<V&>() = std::declval<V&&>()));
}

TEST(OperatorsTest_MoveAssignment, IsNotNoexceptIf_AnyTypeIsThrowingMovable) {
    using V = Variant<ThrowingType>;
    EXPECT_TRUE(!noexcept(std::declval<V&>() = std::declval<V&&>()));
}

TEST(OperatorsTest_MoveAssignment, SelfAssignment_DoesNothing) {
    Variant<int> v(42);
    Variant<int>& ref = v;
    v = std::move(ref);
    EXPECT_EQ(v.get<int>(), 42);
}

TEST(OperatorsTest_MoveAssignment, ValidMove_MovesCorrectly) {
    Variant<int> v1(123), v2;
    v2 = std::move(v1);
    EXPECT_EQ(v2.index(), 0);
    EXPECT_EQ(v2.get<int>(), 123);
}

TEST(OperatorsTest_MoveAssignment, ThrowsAndBecomesValuelessIf_MoveFails) {
    Variant<ThrowingType> v1, v2;
    try {
        v2 = std::move(v1);
    }
    catch (...) {
        EXPECT_TRUE(v2.valueless_by_exception());
    }
}

TEST(OperatorsTest_MoveAssignment, DestroysPreviousTypeAnd_MoveConstruct) {
    Tracker::reset();
    Variant<Tracker, int> v1, v2(std::in_place_index<1>, 5);
    v1 = std::move(v2);
    v2 = Variant<Tracker, int>();
    EXPECT_TRUE(Tracker::destroyed);
    EXPECT_TRUE(Tracker::moved);
    EXPECT_EQ(v2.index(), 0);
}

TEST(OperatorsTest_MoveAssignment, DestroysPreviousTypeAnd_MoveAssignment) {
    Tracker::reset();
    Variant<Tracker, int> v1, v2(std::in_place_index<0>, 5);
    v1 = std::move(v2);
    v2 = Variant<Tracker, int>();
    EXPECT_FALSE(Tracker::destroyed);
    EXPECT_TRUE(Tracker::assignMoved);
    EXPECT_EQ(v2.index(), 0);
    EXPECT_EQ(v2.get<Tracker>(), 0);
}

TEST(OperatorsTest_MoveAssignment, DestroysPreviousTypeWith_ValuelessObj) {
    Tracker::reset();
    Variant<Tracker, ThrowingType> v1, v2;
    try {
        v2 = ThrowingType();
    }
    catch (...) {
        EXPECT_TRUE(v2.valueless_by_exception());
    }
    v1 = v2;
    EXPECT_TRUE(v1.valueless_by_exception());
}


TEST(OperatorsTest_ValueAssignment, FailsIf_TypeIsNotConstructible) {
    Variant<NoOperator> v;
    //v = NoOperator{};
    EXPECT_TRUE(true);
}

TEST(OperatorsTest_ValueAssignment, IsNoexceptIf_TypeIsNoexceptConstructible) {
    using V = Variant<int>;
    EXPECT_TRUE(noexcept(std::declval<V&>() = std::declval<int&>()));
}

TEST(OperatorsTest_ValueAssignment, IsNotNoexceptIf_TypeIsThrowingConstructible) {
    using V = Variant<ThrowingType>;
    EXPECT_TRUE(!noexcept(std::declval<V&>() = std::declval<ThrowingType&>()));
}

TEST(OperatorsTest_ValueAssignment, SelfAssignment_DoesNothing) {
    Variant<int> v(42);
    Variant<int>& ref = v;
    v = std::move(ref.get<int>());
    EXPECT_EQ(v.get<int>(), 42);
}

TEST(OperatorsTest_ValueAssignment, ValidAssignment_AssignsCorrectly) {
    Variant<int> v;
    v = 123;
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<int>(), 123);
}

TEST(OperatorsTest_ValueAssignment, ThrowsAndBecomesValuelessIf_AssignmentFails) {
    Variant<ThrowingType> v;
    try {
        v = ThrowingType();
    }
    catch (...) {
        EXPECT_TRUE(v.valueless_by_exception());
    }
}

TEST(OperatorsTest_ValueAssignment, ThrowsAndBecomesValuelessIf_ConstructionFails) {
    Variant<Tracker, ThrowingType> v;
    try {
        v = ThrowingType();
    }
    catch (...) {
        EXPECT_TRUE(v.valueless_by_exception());
    }
}

TEST(OperatorsTest_ValueAssignment, DestroysPreviousTypeAnd_Assignment) {
    Tracker::reset();
    Variant<Tracker, int> v;
    Tracker t;
    v = t;
    EXPECT_FALSE(Tracker::destroyed);
    EXPECT_TRUE(Tracker::assignCopied);
    EXPECT_EQ(v.index(), 0);
}

TEST(OperatorsTest_ValueAssignment, DestroysPreviousTypeAnd_Construction) {
    Tracker::reset();
    Variant<Tracker, int> v;
    v = 1;
    EXPECT_TRUE(Tracker::destroyed);
    Tracker t;
    v = t;
    EXPECT_FALSE(Tracker::assignCopied);
    EXPECT_TRUE(Tracker::copied);
    EXPECT_EQ(v.index(), 0);
}