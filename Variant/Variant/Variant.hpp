#include <type_traits>
#include <variant>
#include "../../VariadicUnion/VariadicUnion/VariadicUnion.hpp"
#include "../../Auxiliary_meta_functions/Auxiliary_meta_functions/Auxiliary_meta_functions.hpp"


template<typename... Types>
    requires meta_functions::_Is_pack_of_different_type<Types...>&&
             meta_functions::_Is_pack_not_empty<Types...>
class Variant final {
private:
    VariadicUnion<Types...> _storage;
    size_t _index = -1;

    template<size_t I>
    constexpr void validate_access() const {
        if (valueless_by_exception()) {
            throw std::bad_variant_access();
        }
        if (_index != I) {
            throw std::bad_variant_access();
        }
    }

    template<size_t isNoexcept, typename Type>
    constexpr void variant_assign(bool isSameType, size_t otherIndex, Type&& src)
        noexcept(isNoexcept) {
        using Pure_type = std::remove_cvref_t<Type>;
        if (isSameType) {
            if constexpr (isNoexcept) {
                _storage.get<Pure_type>() = std::forward<Type>(src);
            }
            else {
                try {
                    _storage.get<Pure_type>() = std::forward<Type>(src);
                }
                catch (...) {
                    _index = npos;
                    throw;
                }

            }
        }
        else {
            ((meta_functions::_Get_index_v<Types, Types...> == _index ?
                _storage.destroy<Types>()
                : void()), ...);
            _index = otherIndex;
            if constexpr (isNoexcept) {
                _storage.create<Pure_type>(std::forward<Type>(src));
            }
            else {
                try {
                    _storage.create<Pure_type>(std::forward<Type>(src));
                }
                catch (...) {
                    _index = npos;
                    throw;
                }
            }
        }
    }

    template<typename Type, typename Creator>
    constexpr Type& _emplace_impl(std::size_t new_index, Creator&& creator) {
        ((meta_functions::_Get_index_v<Types, Types...> == _index ?
            _storage.destroy<Types>() : void()), ...);

        _index = new_index;

        try {
            std::forward<Creator>(creator)();
        }
        catch (...) {
            _index = npos;
            throw;
        }

        return _storage.get<Type>();
    }

public:
    inline static constexpr std::size_t npos = -1;

    constexpr bool valueless_by_exception() const noexcept {
        return _index == npos;
    }

    constexpr size_t index() const noexcept {
        return _index;
    }

    template<class Type>
        requires meta_functions::_Is_type_present<Type, Types...>
    constexpr bool holds_alternative() const noexcept {
        return meta_functions::_Get_index_v<Type, Types...> == _index;
    }

    constexpr Variant()
        noexcept(std::is_nothrow_default_constructible_v<
            meta_functions::_Get_first_t<Types...>>)
        : _index(0), _storage() {
        static_assert(meta_functions::_First_type_default_constructible<Types...>,
            "First type haven't default constructor");
        _storage.create<meta_functions::_Get_first_t<Types...>>();
    }

    constexpr Variant(const Variant& other)
        noexcept((std::is_nothrow_copy_constructible_v<Types> && ...))
        requires meta_functions::_All_copy_constructible<Types...>
    : _index(other._index), _storage() {
        ((meta_functions::_Get_index_v<Types, Types...> == _index ?
            _storage.create<Types>(
                other._storage.get<Types>())
            : void()), ...);
    }

    constexpr Variant(Variant&& other)
        noexcept((std::is_nothrow_move_constructible_v<Types> && ...))
        requires meta_functions::_All_move_constructible<Types...>
    : _index(other._index), _storage() {
        ((meta_functions::_Get_index_v<Types, Types...> == _index ?
            _storage.create<Types>(
                std::move(other._storage.get<Types>()))
            : void()), ...);
        other._index = npos;
    }

    template<typename Type>
        requires meta_functions::_Is_type_present<std::remove_cvref_t<Type>, Types...>&&
    std::is_constructible_v<std::remove_cvref_t<Type>, Type>
        constexpr Variant(Type&& value)
        noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Type>, Type>)
        : _index(meta_functions::_Get_index_v<std::remove_cvref_t<Type>, Types...>), _storage() {
        using Pure_type = std::remove_cvref_t<Type>;
        _storage.create<Pure_type>(std::forward<Type>(value));
    }

    template<typename Type, typename... Args>
        requires meta_functions::_Is_type_present<Type, Types...>&&
    meta_functions::_Is_constructible_from_args<Type, Args...>
        constexpr explicit Variant(std::in_place_type_t<Type>, Args&&... args)
        : _index(meta_functions::_Get_index_v<Type, Types...>), _storage() {
        _storage.create<Type>(std::forward<Args>(args)...);
    }

    template<typename Type, typename UType, typename... Args>
        requires meta_functions::_Is_type_present<Type, Types...>&&
    meta_functions::_Is_constructible_from_init_list<Type, UType, Args...>
        constexpr explicit Variant(std::in_place_type_t<Type>, std::initializer_list<UType> il, Args&&... args)
        : _index(meta_functions::_Get_index_v<Type, Types...>), _storage() {
        _storage.create<Type>(il, std::forward<Args>(args)...);
    }

    template<size_t I, typename... Args>
        requires meta_functions::_Is_type_present<meta_functions::_Get_type_t<I, Types...>, Types...>&&
    meta_functions::_Is_constructible_from_args< meta_functions::_Get_type_t<I, Types...>, Args...>
        constexpr explicit Variant(std::in_place_index_t<I>, Args&&... args)
        : _index(I), _storage() {
        using Type = meta_functions::_Get_type_t<I, Types...>;
        _storage.create<Type>(std::forward<Args>(args)...);
    }

    template<size_t I, typename UType, typename... Args>
        requires meta_functions::_Is_type_present<meta_functions::_Get_type_t<I, Types...>, Types...>&&
    meta_functions::_Is_constructible_from_init_list<meta_functions::_Get_type_t<I, Types...>, UType, Args...>
        constexpr explicit Variant(std::in_place_index_t<I>, std::initializer_list<UType> il, Args&&... args)
        : _index(I), _storage() {
        using Type = meta_functions::_Get_type_t<I, Types...>;
        _storage.create<Type>(il, std::forward<Args>(args)...);
    }

    constexpr ~Variant() {
        if (!valueless_by_exception()) {
            ((meta_functions::_Get_index_v<Types, Types...> == _index
                ? _storage.destroy<Types>() : void()), ...);
        }
    }

public:
    template<typename Type, typename... Args>
        requires meta_functions::_Is_type_present<Type, Types...>&&
                 meta_functions::_Is_constructible_from_args<Type, Args...>
    constexpr Type& emplace(Args&&... args) {
        constexpr std::size_t new_index = meta_functions::_Get_index_v<Type, Types...>;
        return _emplace_impl<Type>(new_index, [&] {
            _storage.create<Type>(std::forward<Args>(args)...); });
    }

    template<typename Type, typename UType, typename... Args>
        requires meta_functions::_Is_type_present<Type, Types...>&&
                 meta_functions::_Is_constructible_from_init_list<Type, UType, Args...>
    constexpr Type& emplace(std::initializer_list<UType> il, Args&&... args) {
        constexpr std::size_t new_index = meta_functions::_Get_index_v<Type, Types...>;
        return _emplace_impl<Type>(new_index, [&] {
            _storage.create<Type>(il, std::forward<Args>(args)...); });
    }

    template<std::size_t I, typename... Args>
        requires meta_functions::_Is_index_of_alternative<I, Types...>&&
                 meta_functions::_Is_constructible_from_args<meta_functions::_Get_type_t<I, Types...>, Args...>
    constexpr meta_functions::_Get_type_t<I, Types...>& emplace(Args&&... args) {
        using Type = meta_functions::_Get_type_t<I, Types...>;
        return _emplace_impl<Type>(I, [&] {
            _storage.create<Type>(std::forward<Args>(args)...); });
    }

    template<std::size_t I, typename UType, typename... Args>
        requires meta_functions::_Is_index_of_alternative<I, Types...>&&
                 meta_functions::_Is_constructible_from_init_list<meta_functions::_Get_type_t<I, Types...>, UType, Args...>
    constexpr meta_functions::_Get_type_t<I, Types...>& emplace(std::initializer_list<UType> il, Args&&... args) {
        using Type = meta_functions::_Get_type_t<I, Types...>;
        return _emplace_impl<Type>(I, [&] {
            _storage.create<Type>(il, std::forward<Args>(args)...); });
    }

public:
    void swap(Variant& other)
        noexcept(((std::is_nothrow_move_constructible_v<Types>&& 
                   std::is_nothrow_move_assignable_v<Types> &&
                   std::is_nothrow_swappable_v<Types>) && ...))
        requires meta_functions::_All_move_constructible<Types...>&&
                 meta_functions::_All_swappable<Types...>
    {
        if (valueless_by_exception() && other.valueless_by_exception()) {
            return;
        }

        if (_index == other._index) {
            ((meta_functions::_Get_index_v<Types, Types...> == _index ?
                std::swap(_storage.get<Types>(), other._storage.get<Types>())
                : void()), ...);
            return;
        }

        if (valueless_by_exception()) {
            *this = std::move(other);
            other._index = npos;
            return;
        }

        if (other.valueless_by_exception()) {
            other = std::move(*this);
            _index = npos;
            return;
        }

        Variant temp = std::move(*this);
        *this = std::move(other);
        other = std::move(temp);
    }

public:
    constexpr Variant& operator=(const Variant& other)
        noexcept(((std::is_nothrow_copy_constructible_v<Types>&&
                std::is_nothrow_copy_assignable_v<Types>) && ...))
        requires meta_functions::_All_copy_constructible<Types...>&&
                 meta_functions::_All_copy_assignable<Types...>
    {
        if (*this == other) return *this;

        if constexpr (((std::is_trivially_copy_constructible_v<Types> &&
            std::is_trivially_copy_assignable_v<Types> &&
            std::is_trivially_destructible_v<Types>) && ...)) {
            _index = other._index;
            _storage = other._storage;

        }
        else {
            if (other.valueless_by_exception()) {
                ((meta_functions::_Get_index_v<Types, Types...> == _index ?
                    _storage.destroy<Types>()
                    : void()), ...);
                _index = npos;
                return *this;
            }

            constexpr bool isNoexcept = ((std::is_nothrow_copy_constructible_v<Types> &&
                std::is_nothrow_copy_assignable_v<Types>) && ...);

            bool isSameType = (((meta_functions::_Get_index_v<Types, Types...> == other._index ?
                std::is_copy_assignable_v<Types> : 0) + ...) && _index == other._index);

            ((meta_functions::_Get_index_v<Types, Types...> == other._index ?
                variant_assign<isNoexcept, const Types&>(
                    isSameType, other._index, other._storage.get<Types>())
                : void()), ...);
        }
        return *this;
    }


    constexpr Variant& operator=(Variant&& other)
        noexcept(((std::is_nothrow_move_constructible_v<Types>&&
                std::is_nothrow_move_assignable_v<Types>) && ...))
        requires meta_functions::_All_move_constructible<Types...>&&
                 meta_functions::_All_move_assignable<Types...>
    {
        if (*this == other) return *this;

        if constexpr (((std::is_trivially_move_constructible_v<Types> &&
            std::is_trivially_move_assignable_v<Types> &&
            std::is_trivially_destructible_v<Types>) && ...)) {
            _index = other._index;
            _storage = other._storage;

        }
        else {

            if (other.valueless_by_exception()) {
                ((meta_functions::_Get_index_v<Types, Types...> == _index ?
                    _storage.destroy<Types>()
                    : void()), ...);
                _index = npos;
                return *this;
            }

            constexpr bool isNoexcept = ((std::is_nothrow_move_constructible_v<Types> &&
                std::is_nothrow_move_assignable_v<Types>) && ...);

            bool isSameType = (((meta_functions::_Get_index_v<Types, Types...> == other._index ?
                std::is_move_assignable_v<Types> : 0) + ...) && _index == other._index);

            ((meta_functions::_Get_index_v<Types, Types...> == other._index ?
                variant_assign<isNoexcept, Types>(
                    isSameType, other._index, std::move(other._storage.get<Types>()))
                : void()), ...);
        }
        other._index = npos;

        return *this;
    }

    template<typename Type>
        requires meta_functions::_Is_type_present<std::remove_cvref_t<Type>, Types...>&&
                 meta_functions::_Is_constructible_from_itself<Type>&& meta_functions::_Is_assignable<Type>
    constexpr Variant& operator=(Type&& obj)
        noexcept((std::is_nothrow_constructible_v<std::remove_cvref_t<Type>, Type&&>&&
                std::is_nothrow_assignable_v<Type&, Type&&>)) {
        using Pure_type = std::remove_cvref_t<Type>;

        constexpr bool isNoexcept = (std::is_nothrow_constructible_v<Pure_type, Type> &&
            std::is_nothrow_assignable_v<Pure_type, Type>);

        constexpr size_t obj_index = meta_functions::_Get_index_v<Pure_type, Types...>;
        bool isSameType = (_index == obj_index) && std::is_assignable_v<Pure_type, Type>;

        variant_assign<isNoexcept, Type>(isSameType, obj_index, std::forward<Type>(obj));

        return *this;
    }


    constexpr bool operator==(const Variant& other) const
        noexcept((meta_functions::is_nothrow_equality_comparable_v<Types> && ...))
        requires meta_functions::_All_equality_comparable<Types...>
    {

        if (valueless_by_exception() && other.valueless_by_exception()) {
            return true;
        }

        if (valueless_by_exception() != other.valueless_by_exception()) {
            return false;
        }

        if (_index != other._index) {
            return false;
        }

        return (((meta_functions::_Get_index_v<Types, Types...> == _index) ?
            (_storage.get<Types>() == other._storage.get<Types>())
            : false) || ...);
    }

    constexpr bool operator!=(const Variant& other) {
        return !(*this == other);
    }

public:
    template <typename Type>
        requires meta_functions::_Is_type_present<Type, Types...>
    constexpr const Type& get() const& {
        validate_access<meta_functions::_Get_index_v<Type, Types...>>();
        return _storage.get<Type>();
    }

    template <typename Type>
        requires meta_functions::_Is_type_present<Type, Types...>
    constexpr Type& get()& {
        validate_access<meta_functions::_Get_index_v<Type, Types...>>();
        return _storage.get<Type>();
    }

    template <typename Type>
        requires meta_functions::_Is_type_present<Type, Types...>
    constexpr const Type&& get() const&& {
        validate_access<meta_functions::_Get_index_v<Type, Types...>>();
        return std::move(_storage.get<Type>());
    }

    template <typename Type>
        requires meta_functions::_Is_type_present<Type, Types...>
    constexpr Type&& get()&& {
        validate_access<meta_functions::_Get_index_v<Type, Types...>>();
        return std::move(_storage.get<Type>());
    }

    template <size_t I>
        requires meta_functions::_Is_index_of_alternative<I, Types...>
    constexpr const meta_functions::_Get_type_t<I, Types...>& get() const& {
        validate_access<I>();
        return _storage.get<meta_functions::_Get_type_t<I, Types...>>();
    }

    template <size_t I>
        requires meta_functions::_Is_index_of_alternative<I, Types...>
    constexpr meta_functions::_Get_type_t<I, Types...>& get()& {
        validate_access<I>();
        return _storage.get<meta_functions::_Get_type_t<I, Types...>>();
    }

    template <size_t I>
        requires meta_functions::_Is_index_of_alternative<I, Types...>
    constexpr const meta_functions::_Get_type_t<I, Types...>&& get() const&& {
        validate_access<I>();
        return std::move(_storage.get<meta_functions::_Get_type_t<I, Types...>>());
    }

    template <size_t I>
        requires meta_functions::_Is_index_of_alternative<I, Types...>
    constexpr meta_functions::_Get_type_t<I, Types...>&& get()&& {
        validate_access<I>();
        return std::move(_storage.get<meta_functions::_Get_type_t<I, Types...>>());
    }

public:
    template <typename Type>
        requires meta_functions::_Is_type_present<Type, Types...>
    constexpr Type* get_if() & noexcept {
        return (!valueless_by_exception() &&
            _index == meta_functions::_Get_index_v<Type, Types...>)
            ? &_storage.get<Type>()
            : nullptr;
    }

    template <typename Type>
        requires meta_functions::_Is_type_present<Type, Types...>
    constexpr const Type* get_if() const& noexcept {
        return (!valueless_by_exception() &&
            _index == meta_functions::_Get_index_v<Type, Types...>)
            ? &_storage.get<Type>()
            : nullptr;
    }

    template <std::size_t I>
        requires meta_functions::_Is_index_of_alternative<I, Types...>
    constexpr meta_functions::_Get_type_t<I, Types...>* get_if() & noexcept {
        return (!valueless_by_exception() &&
            _index == I)
            ? &_storage.get<meta_functions::_Get_type_t<I, Types...>>()
            : nullptr;
    }

    template <std::size_t I>
        requires meta_functions::_Is_index_of_alternative<I, Types...>
    constexpr const meta_functions::_Get_type_t<I, Types...>* get_if() const& noexcept {
        return (!valueless_by_exception() &&
            _index == I)
            ? &_storage.get<meta_functions::_Get_type_t<I, Types...>>()
            : nullptr;
    }
};
