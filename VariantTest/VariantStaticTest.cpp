#include "pch.h"
#include "Variant.hpp"

TEST(VariantStaticTests, NonEmptyPack) {
    static_assert(detail::_Is_pack_not_empty<int>, "Should pass: one type");
    static_assert(detail::_Is_pack_not_empty<int, double>, "Should pass: multiple types");
    // static_assert(detail::_Is_pack_not_empty<>, "Should fail: empty pack");
}

TEST(VariantStaticTests, DistinctTypes) {
    static_assert(detail::_Is_pack_of_different_type<int, double, char>, "Should pass: all distinct");
    // static_assert(detail::_Is_pack_of_different_type<int, int>, "Should fail: duplicate types");
}
