#pragma once
#include <type_traits>
#include <concepts>
#include <initializer_list>

#include "Getters.hpp"


namespace meta_functions {
    template<typename T, typename... Types>
    concept _Is_type_present = (std::is_same_v<T, Types> || ...);


    template<std::size_t I, typename... Types>
    concept _Is_index_of_alternative = (I < sizeof...(Types));


    template<typename Type, typename... Types>
    constexpr bool _are_unique_impl() {
        if constexpr (sizeof...(Types) == 0) {
            return true;
        }
        else {
            return ((!std::is_same_v<Type, Types>) && ...)
                && _are_unique_impl<Types...>();
        }
    }

    template<typename... Types>
    concept _Is_pack_of_different_type = _are_unique_impl<Types...>();

    template<typename... Types>
    concept _Is_pack_not_empty = sizeof...(Types) > 0;

    template<typename... Types>
    concept _First_type_default_constructible =
        std::is_default_constructible_v<_Get_first_t<Types...>>;

    template<typename... Types>
    concept _All_copy_constructible = (std::is_copy_constructible_v<Types> && ...);

    template<typename... Types>
    concept _All_move_constructible = (std::is_move_constructible_v<Types> && ...);

    template<typename Type>
    concept _Is_constructible_from_itself =
        std::is_constructible_v<std::remove_cvref_t<Type>, Type>;

    template<typename Type, typename... Args>
    concept _Is_constructible_from_args = std::is_constructible_v<Type, Args...>;

    template<typename Type, typename UType, typename... Args>
    concept _Is_constructible_from_init_list = std::is_constructible_v<Type, std::initializer_list<UType>, Args...>;

    template<typename... Types>
    concept _All_swappable = (std::is_swappable_v<Types> && ...);

    template<typename... Types>
    concept _All_copy_assignable = (std::is_copy_assignable_v<Types> && ...);

    template<typename... Types>
    concept _All_move_assignable = (std::is_move_assignable_v<Types> && ...);

    template<typename Type>
    concept _Is_assignable = std::is_assignable_v<std::remove_cvref_t<Type>&, Type>;

    template<typename... Types>
    concept _All_trivially_move_assignable =
        ((std::is_trivially_move_constructible_v<Types> &&
            std::is_trivially_move_assignable_v<Types> &&
            std::is_trivially_destructible_v<Types>) && ...);

    template<typename... Types>
    concept _All_equality_comparable = (std::equality_comparable<Types> && ...);

    template<typename Type>
    constexpr bool is_nothrow_equality_comparable_v =
        noexcept(std::declval<Type>() == std::declval<Type>());
}