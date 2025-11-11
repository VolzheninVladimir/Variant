#include "pch.h"
#include "Variant.hpp"


TEST(ConstexprTest_Helpers, IndexAndValuelessAreConstexpr) {
    constexpr Variant<int, double> v;
    static_assert(v.index() == 0);
    static_assert(!v.valueless_by_exception());
}

TEST(ConstexprTest_Helpers, HoldsAlternativeIsConstexpr) {
    constexpr Variant<int, double> v;
    static_assert(v.holds_alternative<int>());
    static_assert(!v.holds_alternative<double>());
}


TEST(ConstexprTest_Constructors, DefaultConstructorIsConstexpr) {
    constexpr Variant<int, double> v;
    static_assert(v.index() == 0);
    static_assert(v.get<int>() == 0);
}

TEST(ConstexprTest_Constructors, ValueConstructorIsConstexpr) {
    constexpr Variant<int, double> v(3.14);
    static_assert(v.index() == 1);
    static_assert(v.get<double>() == 3.14);
}

TEST(ConstexprTest_Constructors, InPlaceTypeConstructorIsConstexpr) {
    constexpr Variant<int, double> v(std::in_place_type<double>, 2.71);
    static_assert(v.index() == 1);
    static_assert(v.get<double>() == 2.71);
}

TEST(ConstexprTest_Constructors, InPlaceIndexConstructorIsConstexpr) {
    constexpr Variant<int, double> v(std::in_place_index_t<1>{}, 1.618);
    static_assert(v.index() == 1);
    static_assert(v.get<double>() == 1.618);
}

TEST(ConstexprTest_Constructors, CopyConstructorIsConstexpr) {
    constexpr Variant<int, double> original(42);
    constexpr Variant<int, double> copy(original);
    static_assert(copy.index() == 0);
    static_assert(copy.get<int>() == 42);
}

TEST(ConstexprTest_Constructors, MoveConstructorIsConstexpr) {
    constexpr Variant<int, double> original(3.14);
    constexpr Variant<int, double> moved(std::move(original));
    static_assert(moved.index() == 1);
    static_assert(moved.get<double>() == 3.14);
}

struct ConstexprList {
    int sum = 0;
    constexpr ConstexprList(std::initializer_list<int> il) {
        for (int x : il) sum += x;
    }
};

TEST(ConstexprTest_Constructors, InPlaceTypeInitListConstructorIsConstexpr) {
    constexpr std::variant<ConstexprList> v(std::in_place_type<ConstexprList>, { 1, 2, 3 });
    static_assert(v.index() == 0);
    static_assert(std::get<0>(v).sum == 6);
}

TEST(ConstexprTest_Constructors, InPlaceIndexInitListConstructorIsConstexpr) {
    constexpr std::variant<ConstexprList> v(std::in_place_index<0>, { 1, 2, 3 });
    static_assert(v.index() == 0);
    static_assert(std::get<0>(v).sum == 6);
}

TEST(ConstexprTest_Emplace, EmplaceByTypeIsConstexpr) {
    Variant<int, double> v;
    double& ref = v.emplace<double>(42.0);
    static_assert(std::is_same_v<decltype(ref), double&>);
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(ref, 42.0);
}

TEST(ConstexprTest_Emplace, EmplaceByTypeInitListIsConstexpr) {
    Variant<std::vector<int>, double> v;
    auto& ref = v.emplace<std::vector<int>>({ 1, 2, 3 });
    static_assert(std::is_same_v<decltype(ref), std::vector<int>&>);
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(ref.size(), 3);
}

TEST(ConstexprTest_Emplace, EmplaceByIndexIsConstexpr) {
    Variant<int, double> v;
    auto& ref = v.emplace<1>(99.0);
    static_assert(std::is_same_v<decltype(ref), double&>);
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(ref, 99.0);
}

TEST(ConstexprTest_Emplace, EmplaceByIndexInitListIsConstexpr) {
    Variant<std::vector<int>, double> v;
    auto& ref = v.emplace<0>({ 7, 8 });
    static_assert(std::is_same_v<decltype(ref), std::vector<int>&>);
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(ref.size(), 2);
}


TEST(ConstexprTest_Operators, CopyAssignmentIsConstexpr) {
    Variant<int, double> a(42);
    Variant<int, double> b;
    b = a;
    EXPECT_EQ(b.index(), 0);
    EXPECT_EQ(b.get<int>(), 42);
}

TEST(ConstexprTest_Operators, MoveAssignmentIsConstexpr) {
    Variant<int, double> a(3.14);
    Variant<int, double> b;
    b = std::move(a);
    EXPECT_EQ(b.index(), 1);
    EXPECT_EQ(b.get<double>(), 3.14);
}

TEST(ConstexprTest_Operators, ValueAssignmentIsConstexpr) {
    Variant<int, double> v;
    v = 99;
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<int>(), 99);
}

TEST(ConstexprTest_Operators, EqualityOperatorIsConstexpr) {
    constexpr Variant<int, double> a(42);
    constexpr Variant<int, double> b(42);
    static_assert(a == b);
}

TEST(ConstexprTest_Operators, InequalityOperatorIsConstexpr) {
    constexpr Variant<int, double> a(42);
    constexpr Variant<int, double> b(3.14);
    static_assert(a != b);
}


TEST(ConstexprTest_Getters, GetByTypeConstRefIsConstexpr) {
    constexpr Variant<int, double> v(3.14);
    static_assert(v.get<double>() == 3.14);
}

TEST(ConstexprTest_Getters, GetByTypeRefIsConstexpr) {
    Variant<int, double> v(42);
    int& ref = v.get<int>();
    EXPECT_EQ(ref, 42);
}

TEST(ConstexprTest_Getters, GetByTypeRvalueRefIsConstexpr) {
    Variant<int, double> v(42);
    int&& ref = std::move(v).get<int>();
    EXPECT_EQ(ref, 42);
}

TEST(ConstexprTest_Getters, GetByIndexConstRefIsConstexpr) {
    constexpr Variant<int, double> v(3.14);
    static_assert(v.get<1>() == 3.14);
}

TEST(ConstexprTest_Getters, GetByIndexRefIsConstexpr) {
    Variant<int, double> v(42);
    int& ref = v.get<0>();
    EXPECT_EQ(ref, 42);
}

TEST(ConstexprTest_Getters, GetByIndexRvalueRefIsConstexpr) {
    Variant<int, double> v(42);
    int&& ref = std::move(v).get<0>();
    EXPECT_EQ(ref, 42);
}

TEST(ConstexprTest_Getters, GetIfByTypeIsConstexpr) {
    constexpr Variant<int, double> v(3.14);
    static_assert(v.get_if<double>() != nullptr);
    static_assert(v.get_if<int>() == nullptr);
}

TEST(ConstexprTest_Getters, GetIfByIndexIsConstexpr) {
    constexpr Variant<int, double> v(3.14);
    static_assert(v.get_if<1>() != nullptr);
    static_assert(v.get_if<0>() == nullptr);
}
