#pragma once

namespace meta_functions {
    template<size_t I, typename... Types>
    struct _Get_type;

    template<typename First_Type, typename... Types>
    struct _Get_type<0, First_Type, Types...> {
        using Type = typename First_Type;
    };

    template<size_t I, typename First_Type, typename... Types>
    struct _Get_type<I, First_Type, Types...> {
        using Type = typename _Get_type<I - 1, Types...>::Type;
    };

    template<size_t I>
    struct _Get_type<I> {
        static_assert(false, "Type not found");
    };

    template<size_t I, typename... Types>
    using _Get_type_t = typename _Get_type<I, Types...>::Type;


    template<typename First, typename... Types>
    using _Get_first_t = First;


    template<typename Type, typename... Types>
    struct _Get_index;

    template<typename Type, typename Head, typename... Types>
    struct _Get_index<Type, Head, Types...> {
        static constexpr size_t value = 1 + _Get_index<Type, Types...>::value;
    };

    template<typename Type, typename... Types>
    struct _Get_index<Type, Type, Types...> {
        static constexpr size_t value = 0;
    };

    template<typename Type>
    struct _Get_index<Type> {
        static_assert(false, "Index not found");
    };

    template<typename Type, typename... Types>
    static constexpr size_t _Get_index_v = _Get_index<Type, Types...>::value;
}