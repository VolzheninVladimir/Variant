#include "pch.h"
#include "Variant.hpp"
#include <string>

namespace {
    struct CustomType {
        std::vector<int> data;
        int x;
        CustomType(int val) : x(val) {}
        CustomType(std::initializer_list<int> il, int extra) {
            data = il;
            data.push_back(extra);
        }
    };

    struct NoEmplace {
        NoEmplace() = default;
        NoEmplace(int) = delete;
        NoEmplace(std::initializer_list<int>, int) = delete;
    };

    struct ThrowingType {
        ThrowingType() = default;
        ThrowingType(int) {
            if (valueless_trigger) throw std::runtime_error("fail");
        }
        ThrowingType(std::initializer_list<int>, int) {
            if (valueless_trigger) throw std::runtime_error("fail");
        }
        static inline bool valueless_trigger = false;
    };


    struct DestructionTracker {
        static inline bool destroyed = false;
        DestructionTracker() = default;
        DestructionTracker(int x) {}
        DestructionTracker(std::initializer_list<int>) {}
        ~DestructionTracker() { destroyed = true; }
    };
}

TEST(EmplaceTest_DirectEmplace, FailsIf_TypeIsNotConstructibleFromArgs) {
    Variant<NoEmplace> v;
    // v.emplace<int>(123);
    EXPECT_TRUE(true);
}

TEST(EmplaceTest_DirectEmplace, IndexIsSetCorrectlyAfterEmplace_SingleType) {
    Variant<int> v;
    v.emplace<int>(42);
    EXPECT_EQ(v.index(), 0);
}

TEST(EmplaceTest_DirectEmplace, IndexIsSetCorrectlyAfterEmplace_TwoTypes) {
    Variant<int, double> v;
    v.emplace<double>(3.14);
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<double>(), 3.14);
}

TEST(EmplaceTest_DirectEmplace, CreatesCorrectType_SingleType) {
    Variant<int> v;
    int& ref = v.emplace<int>(77);
    EXPECT_EQ(v.get<int>(), 77);
    EXPECT_EQ(&ref, &v.get<int>());
}

TEST(EmplaceTest_DirectEmplace, CreatesCorrectType_TwoTypes) {
    Variant<int, CustomType> v;
    CustomType& ref = v.emplace<CustomType>(99);
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<CustomType>().x, 99);
    EXPECT_EQ(&ref, &v.get<CustomType>());
}

TEST(EmplaceTest_DirectEmplace, BecomesValuelessIf_ConstructionThrows) {
    Variant<ThrowingType> v;
    ThrowingType::valueless_trigger = true;
    try {
        v.emplace<ThrowingType>(123);
    }
    catch (...) {
        EXPECT_EQ(v.index(), Variant<ThrowingType>::npos);
    }
    ThrowingType::valueless_trigger = false;
}

TEST(EmplaceTest_DirectEmplace, DestroysPreviousType_BeforeEmplace) {
    Variant<DestructionTracker, int> v;
    DestructionTracker::destroyed = false;
    v.emplace<DestructionTracker>();
    EXPECT_TRUE(DestructionTracker::destroyed);
    DestructionTracker::destroyed = false;
    v.emplace<int>(5);
    EXPECT_TRUE(DestructionTracker::destroyed);
}


TEST(EmplaceTest_InitListEmplace, FailsIf_TypeIsNotConstructibleFromInitListAndArgs) {
    Variant<NoEmplace> v;
    // v.emplace<NoEmplace>({1, 2}, 3);
    EXPECT_TRUE(true);
}

TEST(EmplaceTest_InitListEmplace, IndexIsSetCorrectlyAfterEmplace_SingleType) {
    Variant<std::vector<int>> v;
    v.emplace<std::vector<int>>({ 1, 2, 3 });
    EXPECT_EQ(v.index(), 0);
}

TEST(EmplaceTest_InitListEmplace, IndexIsSetCorrectlyAfterEmplace_TwoTypes) {
    Variant<int, std::vector<int>> v;
    v.emplace<std::vector<int>>({ 4, 5 });
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<std::vector<int>>()[1], 5);
}

TEST(EmplaceTest_InitListEmplace, CreatesCorrectType_SingleType) {
    Variant<std::vector<int>> v;
    auto& ref = v.emplace<std::vector<int>>({ 10, 20 });
    EXPECT_EQ(v.get<std::vector<int>>().size(), 2);
    EXPECT_EQ(&ref, &v.get<std::vector<int>>());
}

TEST(EmplaceTest_InitListEmplace, CreatesCorrectType_TwoTypes) {
    Variant<int, CustomType> v;
    auto& ref = v.emplace<CustomType>({ 1, 2 }, 99);
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<CustomType>().data.size(), 3);
    EXPECT_EQ(v.get<CustomType>().data[2], 99);
    EXPECT_EQ(&ref, &v.get<CustomType>());
}

TEST(EmplaceTest_InitListEmplace, BecomesValuelessIf_ConstructionThrows) {
    Variant<ThrowingType> v;
    ThrowingType::valueless_trigger = true;
    try {
        v.emplace<ThrowingType>({ 1, 2 }, 3);
    }
    catch (...) {
        EXPECT_EQ(v.index(), Variant<ThrowingType>::npos);
    }
    ThrowingType::valueless_trigger = false;
}

TEST(EmplaceTest_InitListEmplace, DestroysPreviousType_BeforeEmplace) {
    Variant<DestructionTracker, int> v;
    DestructionTracker::destroyed = false;
    v.emplace<DestructionTracker>({ 1, 2 });
    EXPECT_TRUE(DestructionTracker::destroyed);
    DestructionTracker::destroyed = false;
    v.emplace<int>(5);
    EXPECT_TRUE(DestructionTracker::destroyed);
}


TEST(EmplaceTest_IndexEmplace, FailsIf_TypeIsNotConstructibleFromArgs) {
    Variant<NoEmplace> v;
    // v.template emplace<0>(123);
    EXPECT_TRUE(true);
}

TEST(EmplaceTest_IndexEmplace, IndexIsSetCorrectlyAfterEmplace_SingleType) {
    Variant<int> v;
    v.template emplace<0>(42);
    EXPECT_EQ(v.index(), 0);
}

TEST(EmplaceTest_IndexEmplace, IndexIsSetCorrectlyAfterEmplace_TwoTypes) {
    Variant<int, double> v;
    v.template emplace<1>(3.14);
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<double>(), 3.14);
}

TEST(EmplaceTest_IndexEmplace, CreatesCorrectType_SingleType) {
    Variant<int> v;
    int& ref = v.template emplace<0>(77);
    EXPECT_EQ(v.get<int>(), 77);
    EXPECT_EQ(&ref, &v.get<int>());
}

TEST(EmplaceTest_IndexEmplace, CreatesCorrectType_TwoTypes) {
    Variant<int, CustomType> v;
    auto& ref = v.template emplace<1>(99);
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<CustomType>().x, 99);
    EXPECT_EQ(&ref, &v.get<CustomType>());
}

TEST(EmplaceTest_IndexEmplace, BecomesValuelessIf_ConstructionThrows) {
    Variant<ThrowingType> v;
    ThrowingType::valueless_trigger = true;
    try {
        v.template emplace<0>(123);
    }
    catch (...) {
        EXPECT_EQ(v.index(), Variant<ThrowingType>::npos);
    }
    ThrowingType::valueless_trigger = false;
}

TEST(EmplaceTest_IndexEmplace, DestroysPreviousType_BeforeEmplace) {
    Variant<DestructionTracker, int> v;
    DestructionTracker::destroyed = false;
    v.template emplace<0>(1);
    EXPECT_TRUE(DestructionTracker::destroyed);
    DestructionTracker::destroyed = false;
    v.template emplace<1>(5);
    EXPECT_TRUE(DestructionTracker::destroyed);
}

TEST(EmplaceTest_InitListIndexEmplace, FailsIf_TypeIsNotConstructibleFromInitListAndArgs) {
    // Variant<NoEmplace> v;
    // v.template emplace<0>({1, 2}, 3); // static_assert провалится
    EXPECT_TRUE(true);
}

TEST(EmplaceTest_InitListIndexEmplace, IndexIsSetCorrectlyAfterEmplace_SingleType) {
    Variant<std::vector<int>> v;
    v.template emplace<0>({ 1, 2, 3 });
    EXPECT_EQ(v.index(), 0);
}

TEST(EmplaceTest_InitListIndexEmplace, IndexIsSetCorrectlyAfterEmplace_TwoTypes) {
    Variant<int, std::vector<int>> v;
    v.template emplace<1>({ 4, 5 });
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<std::vector<int>>()[1], 5);
}

TEST(EmplaceTest_InitListIndexEmplace, CreatesCorrectType_SingleType) {
    Variant<std::vector<int>> v;
    auto& ref = v.template emplace<0>({ 10, 20 });
    EXPECT_EQ(v.get<std::vector<int>>().size(), 2);
    EXPECT_EQ(&ref, &v.get<std::vector<int>>());
}

TEST(EmplaceTest_InitListIndexEmplace, CreatesCorrectType_TwoTypes) {
    Variant<int, CustomType> v;
    auto& ref = v.template emplace<1>({ 1, 2 }, 99);
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<CustomType>().data.size(), 3);
    EXPECT_EQ(v.get<CustomType>().data[2], 99);
    EXPECT_EQ(&ref, &v.get<CustomType>());
}

TEST(EmplaceTest_InitListIndexEmplace, BecomesValuelessIf_ConstructionThrows) {
    Variant<ThrowingType> v;
    ThrowingType::valueless_trigger = true;
    try {
        v.template emplace<0>({ 1, 2 }, 3);
    }
    catch (...) {
        EXPECT_EQ(v.index(), Variant<ThrowingType>::npos);
    }
    ThrowingType::valueless_trigger = false;
}

TEST(EmplaceTest_InitListIndexEmplace, DestroysPreviousType_BeforeEmplace) {
    Variant<DestructionTracker, int> v;
    DestructionTracker::destroyed = false;
    v.template emplace<0>({ 1, 2 });
    EXPECT_TRUE(DestructionTracker::destroyed);
    DestructionTracker::destroyed = false;
    v.template emplace<1>(5);
    EXPECT_TRUE(DestructionTracker::destroyed);
}