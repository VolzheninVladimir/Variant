#include "pch.h"
#include "Auxiliary_meta_functions.hpp"

using namespace meta_functions;

struct ThrowingType {
    ThrowingType() = delete;
    ThrowingType(const ThrowingType&) = delete;
    ThrowingType(ThrowingType&&) noexcept(false) {}
    ThrowingType& operator=(const ThrowingType&) noexcept(false) { return *this; }
    ThrowingType& operator=(ThrowingType&&) noexcept(false) { return *this; }
    bool operator==(const ThrowingType&) const noexcept(false) { return true; }
};

struct A {
    int a;
};

struct B {
    int b;
};

struct NoMoveConstructor {
    NoMoveConstructor(NoMoveConstructor&&) = delete;
};

struct NoMoveAssign {
    NoMoveAssign& operator=(NoMoveAssign&&) = delete;
};

struct NoCopyAssign {
    NoCopyAssign& operator=(const NoCopyAssign&) = delete;
};

struct NoEqual {
    bool operator==(const NoEqual&) = delete;
};

TEST(MetaFunctionsTest_Getters, ReturnsCorrectTypeByIndex) {
    using T0 = _Get_type_t<0, int, double, char>;
    using T1 = _Get_type_t<1, int, double, char>;
    using T2 = _Get_type_t<2, int, double, char>;

    EXPECT_TRUE((std::is_same_v<T0, int>));
    EXPECT_TRUE((std::is_same_v<T1, double>));
    EXPECT_TRUE((std::is_same_v<T2, char>));
}

TEST(MetaFunctionsTest_Getters, ReturnsFirstType) {
    using T = _Get_first_t<int, double, char>;
    EXPECT_TRUE((std::is_same_v<T, int>));
}

TEST(MetaFunctionsTest_Getters, ReturnsCorrectIndexForType) {
    constexpr size_t i0 = _Get_index_v<int, int, double, char>;
    constexpr size_t i1 = _Get_index_v<double, int, double, char>;
    constexpr size_t i2 = _Get_index_v<char, int, double, char>;

    EXPECT_EQ(i0, 0);
    EXPECT_EQ(i1, 1);
    EXPECT_EQ(i2, 2);
}

TEST(MetaFunctionsTest_Traits, DetectsPresenceCorrectly) {
    constexpr bool present = _Is_type_present<int, double, int, char>;
    constexpr bool absent = _Is_type_present<float, double, int, char>;

    EXPECT_TRUE(present);
    EXPECT_FALSE(absent);
}

TEST(MetaFunctionsTest_Traits, ValidatesIndexBounds) {
    EXPECT_TRUE((_Is_index_of_alternative<0, int, double, char>));
    EXPECT_TRUE((_Is_index_of_alternative<2, int, double, char>));
    EXPECT_FALSE((_Is_index_of_alternative<3, int, double, char>));
}

TEST(MetaFunctionsTest_Traits, DetectsUniqueness) {
    EXPECT_TRUE((_Is_pack_of_different_type<int, double, char>));
    EXPECT_FALSE((_Is_pack_of_different_type<int, double, int>));
}

TEST(MetaFunctionsTest_Traits, DetectsNonEmptyPack) {
    EXPECT_TRUE((_Is_pack_not_empty<int>));
    EXPECT_FALSE((_Is_pack_not_empty<>));
}

TEST(MetaFunctionsTest_Traits, ValidatesDefaultConstructibility) {
    EXPECT_TRUE((_First_type_default_constructible<int, double>));
    EXPECT_FALSE((_First_type_default_constructible<ThrowingType, int>));
}

TEST(MetaFunctionsTest_Traits, ValidatesCopyConstructibility) {
    EXPECT_TRUE((_All_copy_constructible<A, B>));
    EXPECT_FALSE((_All_copy_constructible<A, ThrowingType>));
}

TEST(MetaFunctionsTest_Traits, ValidatesMoveConstructibility) {
    EXPECT_TRUE((_All_move_constructible<int, double>));
    EXPECT_FALSE((_All_move_constructible<int, NoMoveConstructor>));
}

TEST(MetaFunctionsTest_Traits, ValidatesSelfConstructibility) {
    EXPECT_TRUE((_Is_constructible_from_itself<int>));
    EXPECT_TRUE((_Is_constructible_from_itself<int&>));
    EXPECT_TRUE((_Is_constructible_from_itself<int&&>));
}

TEST(MetaFunctionsTest_Traits, ValidatesArgsConstructibility) {
    EXPECT_TRUE((_Is_constructible_from_args<int, int>));
    EXPECT_FALSE((_Is_constructible_from_args<ThrowingType, int>));
}

TEST(MetaFunctionsTest_Traits, ValidatesInitListConstructibility) {
    EXPECT_TRUE((_Is_constructible_from_init_list<std::vector<int>, int>));
    EXPECT_FALSE((_Is_constructible_from_init_list<ThrowingType, int>));
}

TEST(MetaFunctionsTest_Traits, ValidatesSwappability) {
    EXPECT_TRUE((_All_swappable<int, double>));
    EXPECT_FALSE((_All_swappable<NoMoveConstructor>));
}

TEST(MetaFunctionsTest_Traits, ValidatesCopyAssignability) {
    EXPECT_TRUE((_All_copy_assignable<int, double>));
    EXPECT_FALSE((_All_copy_assignable<NoCopyAssign>));
}

TEST(MetaFunctionsTest_Traits, ValidatesMoveAssignability) {
    EXPECT_TRUE((_All_move_assignable<int, double>));
    EXPECT_FALSE((_All_move_assignable<NoMoveAssign>));
}

TEST(MetaFunctionsTest_Traits, ValidatesSingleAssignability) {
    EXPECT_TRUE((_Is_assignable<int>));
    EXPECT_FALSE((_Is_assignable<NoMoveAssign>));
}

TEST(MetaFunctionsTest_Traits, ValidatesTrivialMoveAssignability) {
    EXPECT_TRUE((_All_trivially_move_assignable<int, double>));
    EXPECT_FALSE((_All_trivially_move_assignable<ThrowingType>));
}

TEST(MetaFunctionsTest_Traits, ValidatesEqualityComparability) {
    EXPECT_TRUE((_All_equality_comparable<int, double>));
    EXPECT_FALSE((_All_equality_comparable<NoEqual>));
}

TEST(MetaFunctionsTest_Traits, ValidatesNoexceptEquality) {
    EXPECT_TRUE((is_nothrow_equality_comparable_v<int>));
    EXPECT_FALSE((is_nothrow_equality_comparable_v<ThrowingType>));
}