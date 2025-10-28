#include "pch.h"
#include "Variant.hpp"
#include <string>

TEST(ConstructorsTest, DefaultConstructsFirstType) {
    Variant<int, std::string> v;
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<int>(), 0);
}

TEST(ConstructorsTest, DefaultFailsIfFirstTypeNotDefaultConstructible) {
    struct NoDefault {
        NoDefault(int) {}
    };
    // static_assert should trigger at compile time
    // Variant<NoDefault, int> v;
}

TEST(ConstructorsTest, CopyConstructsSameAlternative) {
    Variant<int, std::string> v1(std::in_place_type<std::string>, "hello");
    Variant<int, std::string> v2(v1);
    EXPECT_EQ(v2.index(), 1);
    EXPECT_EQ(v2.get<std::string>(), "hello");
}

TEST(ConstructorsTest, CopyFailsIfTypeNotCopyConstructible) {
    struct NoCopy {
        NoCopy() = default;
        NoCopy(const NoCopy&) = delete;
    };
    // Variant<NoCopy> v1; Variant<NoCopy> v2(v1);
}

TEST(ConstructorsTest, MoveConstructsSameAlternative) {
    Variant<int, std::string> v1(std::in_place_type<std::string>, "world");
    Variant<int, std::string> v2(std::move(v1));
    EXPECT_EQ(v2.index(), 1);
    EXPECT_EQ(v2.get<std::string>(), "world");
}

TEST(ConstructorsTest, MoveFailsIfTypeNotMoveConstructible) {
    struct NoMove {
        NoMove() = default;
        NoMove(NoMove&&) = delete;
    };
    // Variant<NoMove> v1; Variant<NoMove> v2(std::move(v1));
}

TEST(ConstructorsTest, ConstructsFromValue) {
    Variant<int, std::string> v(std::string("test"));
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<std::string>(), "test");
}

TEST(ConstructorsTest, FailsIfTypeNotConstructibleFromValue) {
    struct NoMatch {
        NoMatch(double) {}
    };
    // Variant<NoMatch> v("abc");
}

TEST(ConstructorsTest, InPlaceTypeConstructsCorrectly) {
    Variant<int, std::string> v(std::in_place_type<std::string>, 5, 'a');
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<std::string>(), std::string(5, 'a'));
}

TEST(ConstructorsTest, InPlaceTypeFailsIfArgsInvalid) {
    // Variant<int, std::string> v(std::in_place_type<std::string>, 5, 5.5); // ? не компилируется
}

TEST(ConstructorsTest, InPlaceTypeWithInitListConstructs) {
    Variant<std::vector<int>> v(std::in_place_type<std::vector<int>>, { 1, 2, 3 });
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<std::vector<int>>().size(), 3);
}

TEST(ConstructorsTest, InPlaceTypeWithInitListFailsIfInvalid) {
    // Variant<std::vector<int>> v(std::in_place_type<std::vector<int>>, {"a", "b"}); // ? не компилируется
}

TEST(ConstructorsTest, InPlaceIndexConstructsCorrectAlternative) {
    Variant<int, std::string> v(std::in_place_index<1>, "indexed");
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<1>(), "indexed");
}

TEST(ConstructorsTest, InPlaceIndexFailsIfArgsInvalid) {
    // Variant<int, std::string> v(std::in_place_index<1>, 3.14); // ? не компилируется
}

TEST(ConstructorsTest, InPlaceIndexWithInitListConstructs) {
    Variant<std::vector<int>, std::string> v(std::in_place_index<0>, { 4, 5, 6 });
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<0>().size(), 3);
}

TEST(ConstructorsTest, InPlaceIndexWithInitListFailsIfInvalid) {
    // Variant<std::vector<int>> v(std::in_place_index<0>, {"x", "y"}); // ? не компилируется
}

