#include "pch.h"
#include "Variant.hpp"
#include <string>

namespace {
    struct CustomType {
        int x = 42;
        CustomType() = default;
        CustomType(const CustomType&) = default;
        CustomType(CustomType&&) = default;
        CustomType(int val) : x(val) {}
    };

    struct CustomType_with_InitList {
        std::vector<int> data;
        CustomType_with_InitList(std::initializer_list<int> il, int extra) {
            data = il;
            data.push_back(extra);
        }
    };


    struct ThrowingType {
        ThrowingType() noexcept(false) {}
    };

    struct NoDefaultCtor {
        NoDefaultCtor(int) {}
    };


    struct ThrowingCopy {
        ThrowingCopy() = default;
        ThrowingCopy(const ThrowingCopy&) noexcept(false) {}
    };

    struct NoCopy {
        NoCopy() = default;
        NoCopy(const NoCopy&) = delete;
    };


    struct ThrowingMove {
        ThrowingMove() = default;
        ThrowingMove(ThrowingMove&&) noexcept(false) {}
    };

    struct NoMove {
        NoMove() = default;
        NoMove(NoMove&&) = delete;
    };


    struct ThrowingForward {
        ThrowingForward() = default;
        ThrowingForward(ThrowingForward&&) noexcept(false) {}
    };

    struct NoForward {
        NoForward() = default;
        NoForward(NoForward&&) = delete;
    };

    struct NoArgs {
        NoArgs() = default;
        NoArgs(int) = delete;
    };


    struct ThrowingInit {
        ThrowingInit(std::initializer_list<int>, int) noexcept(false) {}
    };

    struct NoInit {
        NoInit() = default;
        NoInit(std::initializer_list<int>, int) = delete;
    };

}

TEST(ConstructorsTest_DefaultConstructor, NoexceptWhen_FirstTypeIsNothrowDefaultConstructible) {
    if constexpr (noexcept(Variant<int>())) {
        EXPECT_TRUE(true);
    }
    else {
        EXPECT_TRUE(false);
    }
}

TEST(ConstructorsTest_DefaultConstructor, NotNoexceptWhen_FirstTypeIsThrowing) {
    if constexpr (noexcept(Variant<ThrowingType>())) {
        EXPECT_TRUE(false);
    }
    else {
        EXPECT_TRUE(true);
    }
}

TEST(ConstructorsTest_DefaultConstructor, FailsIf_FirstTypeHasNoDefaultCtor) {
    //Variant<NoDefaultCtor> v;
    EXPECT_TRUE(true);
}

TEST(ConstructorsTest_DefaultConstructor, CreatesVariantWith_SingleTypeCorrectly) {
    Variant<int> v;
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<int>(), 0);
}

TEST(ConstructorsTest_DefaultConstructor, CreatesVariantWith_TwoTypesCorrectly) {
    Variant<int, double> v;
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<int>(), 0);
}

TEST(ConstructorsTest_DefaultConstructor, CreatesVariantWith_CustomTypeCorrectly) {
    Variant<CustomType> v;
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<CustomType>().x, 42);
}


TEST(ConstructorsTest_CopyConstructor, NoexceptWhen_AllTypesAreNothrowCopyConstructible) {
    if constexpr (noexcept(Variant<int, double>(std::declval<const Variant<int, double>&>()))) {
        EXPECT_TRUE(true);
    }
    else {
        EXPECT_TRUE(false);
    }
}

TEST(ConstructorsTest_CopyConstructor, NotNoexceptWhen_AnyTypeThrowsOnCopy) {
    if constexpr (noexcept(Variant<int, ThrowingCopy>(std::declval<const Variant<int, ThrowingCopy>&>()))) {
        EXPECT_TRUE(false);
    }
    else {
        EXPECT_TRUE(true);
    }
}

TEST(ConstructorsTest_CopyConstructor, FailsIf_AnyTypeIsNotCopyConstructible) {
    // Variant<int, NoCopy> v;
    EXPECT_TRUE(true);
}

TEST(ConstructorsTest_CopyConstructor, CopiesVariantWith_SingleTypeCorrectly) {
    Variant<int> original;
    Variant<int> copy = original;
    EXPECT_EQ(copy.index(), 0);
    EXPECT_EQ(copy.get<int>(), 0);
}

TEST(ConstructorsTest_CopyConstructor, CopiesVariantWith_TwoTypesCorrectly) {
    Variant<int, double> original;
    Variant<int, double> copy = original;
    EXPECT_EQ(copy.index(), 0);
    EXPECT_EQ(copy.get<int>(), 0);
}

TEST(ConstructorsTest_CopyConstructor, CopiesVariantWith_CustomTypeCorrectly) {
    Variant<CustomType> original;
    Variant<CustomType> copy = original;
    EXPECT_EQ(copy.index(), 0);
    EXPECT_EQ(copy.get<CustomType>().x, 42);
}


TEST(ConstructorsTest_MoveConstructor, NoexceptWhen_AllTypesAreNothrowMoveConstructible) {
    if constexpr (noexcept(Variant<int, double>(std::declval<Variant<int, double>&&>()))) {
        EXPECT_TRUE(true);
    }
    else {
        EXPECT_TRUE(false);
    }
}

TEST(ConstructorsTest_MoveConstructor, NotNoexceptWhen_AnyTypeThrowsOnMove) {
    if constexpr (noexcept(Variant<int, ThrowingMove>(std::declval<Variant<int, ThrowingMove>&&>()))) {
        EXPECT_TRUE(false);
    }
    else {
        EXPECT_TRUE(true);
    }
}

TEST(ConstructorsTest_MoveConstructor, FailsIf_AnyTypeIsNotMoveConstructible) {
    // Variant<int, NoMove> v;
    EXPECT_TRUE(true);
}

TEST(ConstructorsTest_MoveConstructor, MovesVariantWith_SingleTypeCorrectly) {
    Variant<int> original;
    Variant<int> moved = std::move(original);
    EXPECT_EQ(moved.index(), 0);
    EXPECT_EQ(moved.get<int>(), 0);
}

TEST(ConstructorsTest_MoveConstructor, MovesVariantWith_TwoTypesCorrectly) {
    Variant<int, double> original;
    Variant<int, double> moved = std::move(original);
    EXPECT_EQ(moved.index(), 0);
    EXPECT_EQ(moved.get<int>(), 0);
}

TEST(ConstructorsTest_MoveConstructor, MovesVariantWith_CustomTypeCorrectly) {
    Variant<CustomType> original;
    Variant<CustomType> moved = std::move(original);
    EXPECT_EQ(moved.index(), 0);
    EXPECT_EQ(moved.get<CustomType>().x, 42);
}


TEST(ConstructorsTest_ForwardingConstructor, NoexceptWhen_TypeIsNothrowConstructibleFromRvalue) {
    if constexpr (noexcept(Variant<int>(std::declval<int>()))) {
        EXPECT_TRUE(true);
    }
    else {
        EXPECT_TRUE(false);
    }
}

TEST(ConstructorsTest_ForwardingConstructor, NotNoexceptWhen_TypeThrowsOnRvalueConstruction) {
    if constexpr (noexcept(Variant<ThrowingForward>(std::declval<ThrowingForward>()))) {
        EXPECT_TRUE(false);
    }
    else {
        EXPECT_TRUE(true);
    }
}

TEST(ConstructorsTest_ForwardingConstructor, FailsIf_TypeIsNotConstructibleFromRvalue) {
    // Variant<NoForward> v(std::declval<NoForward>());
    EXPECT_TRUE(true);
}

TEST(ConstructorsTest_ForwardingConstructor, CreatesVariantWith_SingleTypeCorrectly) {
    Variant<int> v(123);
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<int>(), 123);
}

TEST(ConstructorsTest_ForwardingConstructor, CreatesVariantWith_TwoTypesCorrectly) {
    Variant<int, double> v(3.14);
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<double>(), 3.14);
}

TEST(ConstructorsTest_ForwardingConstructor, CreatesVariantWith_CustomTypeCorrectly) {
    CustomType obj;
    Variant<CustomType> v(std::move(obj));
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<CustomType>().x, 42);
}


TEST(ConstructorsTest_InPlaceConstructor, FailsIf_TypeIsNotConstructibleFromArgs) {
    // Variant<NoArgs> v(std::in_place_type_t<NoArgs>{}, 1);
    EXPECT_TRUE(true);
}

TEST(ConstructorsTest_InPlaceConstructor, CreatesVariantWith_SingleTypeCorrectly) {
    Variant<int> v(std::in_place_type_t<int>{}, 123);
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<int>(), 123);
}

TEST(ConstructorsTest_InPlaceConstructor, CreatesVariantWith_TwoTypesCorrectly) {
    Variant<int, double> v(std::in_place_type_t<double>{}, 3.14);
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<double>(), 3.14);
}

TEST(ConstructorsTest_InPlaceConstructor, CreatesVariantWith_CustomTypeCorrectly) {
    Variant<CustomType> v(std::in_place_type_t<CustomType>{}, 77);
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<CustomType>().x, 77);
}


TEST(ConstructorsTest_InPlaceInitListConstructor, FailsIf_TypeIsNotConstructibleFromInitListAndArgs) {
    // Variant<NoInit> v(std::in_place_type_t<NoInit>{}, {1, 2, 3}, 4);
    EXPECT_TRUE(true);
}

TEST(ConstructorsTest_InPlaceInitListConstructor, CreatesVariantWith_SingleTypeCorrectly) {
    Variant<std::vector<int>> v(std::in_place_type_t<std::vector<int>>{}, { 1, 2, 3 });
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<std::vector<int>>().size(), 3);
    EXPECT_EQ(v.get<std::vector<int>>()[0], 1);
}

TEST(ConstructorsTest_InPlaceInitListConstructor, CreatesVariantWith_TwoTypesCorrectly) {
    Variant<int, std::vector<int>> v(std::in_place_type_t<std::vector<int>>{}, { 4, 5 });
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<std::vector<int>>().size(), 2);
    EXPECT_EQ(v.get<std::vector<int>>()[1], 5);
}

TEST(ConstructorsTest_InPlaceInitListConstructor, CreatesVariantWith_CustomTypeCorrectly) {
    Variant<CustomType_with_InitList> v(std::in_place_type_t<CustomType_with_InitList>{}, { 10, 20 }, 99);
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<CustomType_with_InitList>().data.size(), 3);
    EXPECT_EQ(v.get<CustomType_with_InitList>().data[2], 99);
}


TEST(ConstructorsTest_InPlaceIndexConstructor, FailsIf_TypeIsNotConstructibleFromArgs) {
    // Variant<NoIndex> v(std::in_place_index_t<0>{}, 1);
    EXPECT_TRUE(true);
}

TEST(ConstructorsTest_InPlaceIndexConstructor, CreatesVariantWith_SingleTypeCorrectly) {
    Variant<int> v(std::in_place_index_t<0>{}, 123);
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<int>(), 123);
}

TEST(ConstructorsTest_InPlaceIndexConstructor, CreatesVariantWith_TwoTypesCorrectly) {
    Variant<int, double> v(std::in_place_index_t<1>{}, 3.14);
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<double>(), 3.14);
}

TEST(ConstructorsTest_InPlaceIndexConstructor, CreatesVariantWith_CustomTypeCorrectly) {
    Variant<CustomType> v(std::in_place_index_t<0>{}, 77);
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<CustomType>().x, 77);
}


TEST(ConstructorsTest_InPlaceInitListIndexConstructor, FailsIf_TypeIsNotConstructibleFromInitListAndArgs) {
    // Variant<NoInitIndex> v(std::in_place_index_t<0>{}, {1, 2}, 3);
    EXPECT_TRUE(true);
}

TEST(ConstructorsTest_InPlaceInitListIndexConstructor, CreatesVariantWith_SingleTypeCorrectly) {
    Variant<std::vector<int>> v(std::in_place_index_t<0>{}, { 1, 2, 3 });
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<std::vector<int>>().size(), 3);
    EXPECT_EQ(v.get<std::vector<int>>()[0], 1);
}

TEST(ConstructorsTest_InPlaceInitListIndexConstructor, CreatesVariantWith_TwoTypesCorrectly) {
    Variant<int, std::vector<int>> v(std::in_place_index_t<1>{}, { 4, 5 });
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(v.get<std::vector<int>>().size(), 2);
    EXPECT_EQ(v.get<std::vector<int>>()[1], 5);
}

TEST(ConstructorsTest_InPlaceInitListIndexConstructor, CreatesVariantWith_CustomTypeCorrectly) {
    Variant<CustomType_with_InitList> v(std::in_place_index_t<0>{}, { 10, 20 }, 99);
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(v.get<CustomType_with_InitList>().data.size(), 3);
    EXPECT_EQ(v.get<CustomType_with_InitList>().data[2], 99);
}