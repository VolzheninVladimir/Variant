#pragma once
#include "../../Auxiliary_meta_functions/Auxiliary_meta_functions/Auxiliary_meta_functions.hpp"

template<typename... Types>
union VariadicUnion {};

template<typename Head, typename... Tail>
    requires meta_functions::_Is_pack_of_different_type<Head, Tail...>
union VariadicUnion<Head, Tail...> {
    Head head;
    VariadicUnion<Tail...> tail;
public:
    constexpr VariadicUnion() {}

    constexpr ~VariadicUnion() {}

    template<typename Type, typename... Args>
        requires meta_functions::_Is_type_present<Type, Head, Tail...> &&
                 meta_functions::_Is_constructible_from_args<Type, Args...>
    constexpr void create(Args&&... args) {
        if constexpr (std::is_same_v<Type, Head>) {
            std::construct_at(&head, std::forward<Args>(args)...);
        }
        else {
            tail.create<Type>(std::forward<Args>(args)...);
        }
    }

    template<typename Type>
        requires meta_functions::_Is_type_present<Type, Head, Tail...>
    constexpr void destroy() {
        if constexpr (std::is_same_v<Type, Head>) {
            head.~Head();
        }
        else {
            tail.destroy<Type>();
        }
    }

    template<typename Type>
        requires meta_functions::_Is_type_present<Type, Head, Tail...>
    constexpr Type& get() {
        if constexpr (std::is_same_v<Type, Head>) {
            return head;
        }
        else {
            return tail.get<Type>();
        }
    }

    template<typename Type>
        requires meta_functions::_Is_type_present<Type, Head, Tail...>
    constexpr const Type& get() const {
        if constexpr (std::is_same_v<Type, Head>) {
            return head;
        }
        else {
            return tail.get<Type>();
        }
    }
};