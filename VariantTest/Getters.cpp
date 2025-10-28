#include "pch.h"
#include "Variant.hpp"
#include <string>


TEST(GetTest, GetByTypeLvalue) {
    Variant<int, std::string> v(std::in_place_type<std::string>, "abc");
    std::string& ref = v.get<std::string>();
    EXPECT_EQ(ref, "abc");
}

TEST(GetTest, GetByTypeConstLvalue) {
    const Variant<int, std::string> v(std::in_place_type<std::string>, "xyz");
    const std::string& ref = v.get<std::string>();
    EXPECT_EQ(ref, "xyz");
}

TEST(GetTest, GetByTypeRvalue) {
    Variant<int, std::string> v(std::in_place_type<std::string>, "move");
    std::string&& ref = std::move(v).get<std::string>();
    EXPECT_EQ(ref, "move");
}

TEST(GetTest, GetByTypeConstRvalue) {
    const Variant<int, std::string> v(std::in_place_type<std::string>, "constmove");
    const std::string&& ref = std::move(v).get<std::string>();
    EXPECT_EQ(ref, "constmove");
}


TEST(GetTest, GetByIndexLvalue) {
    Variant<int, std::string> v(std::in_place_index<1>, "index");
    auto& ref = v.get<1>();
    EXPECT_EQ(ref, "index");
}

TEST(GetTest, GetByIndexConstLvalue) {
    const Variant<int, std::string> v(std::in_place_index<1>, "constindex");
    const auto& ref = v.get<1>();
    EXPECT_EQ(ref, "constindex");
}

TEST(GetTest, GetByIndexRvalue) {
    Variant<int, std::string> v(std::in_place_index<1>, "rvalindex");
    auto&& ref = std::move(v).get<1>();
    EXPECT_EQ(ref, "rvalindex");
}

TEST(GetTest, GetByIndexConstRvalue) {
    const Variant<int, std::string> v(std::in_place_index<1>, "constrvalindex");
    const auto&& ref = std::move(v).get<1>();
    EXPECT_EQ(ref, "constrvalindex");
}


TEST(Get_ifTest, GetIfByTypeLvalue) {
    Variant<int, std::string> v(std::in_place_type<std::string>, "abc");
    std::string* ptr = v.get_if<std::string>();
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, "abc");
}

TEST(Get_ifTest, GetIfByTypeConstLvalue) {
    const Variant<int, std::string> v(std::in_place_type<std::string>, "xyz");
    const std::string* ptr = v.get_if<std::string>();
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, "xyz");
}


TEST(Get_ifTest, GetIfByIndexLvalue) {
    Variant<int, std::string> v(std::in_place_index<1>, "index");
    auto* ptr = v.get_if<1>();
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, "index");
}

TEST(Get_ifTest, GetIfByIndexConstLvalue) {
    const Variant<int, std::string> v(std::in_place_index<1>, "constindex");
    const auto* ptr = v.get_if<1>();
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, "constindex");
}
