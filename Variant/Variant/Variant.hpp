#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <variant>
#include <iostream>
#include "../Detail/Detail.hpp"


template <typename... Types>
    requires detail::_Is_pack_of_different_type<Types...> &&
             detail::_Is_pack_not_empty<Types...>
class Variant final {
private:
    class Storage final {
    private:
        static constexpr size_t _max_size = std::max({ sizeof(Types)... });
        alignas(std::max({ alignof(Types)... })) char _buffer[_max_size];

    public:
        template <typename Type, typename... Args>
        void create(Args&&... args) {
            new (_buffer) Type(std::forward<Args>(args)...);
        }

        template <typename Type>
        void destroy() {
            reinterpret_cast<Type*>(_buffer)->~Type();
        }

        template <typename Type>
        const Type& get() const {
            return *(reinterpret_cast<const Type*>(_buffer));
        }

        template <typename Type>
        Type& get() {
            return *(reinterpret_cast<Type*>(_buffer));
        }
    };

    static constexpr size_t _count = sizeof...(Types);
    size_t _index;
    Storage _storage;

    template<typename Type>
    void validate_access() const {
        if (valueless_by_exception()) {
            throw std::bad_variant_access();
        }
        if (_index != detail::_Get_index_v<Type, Types...>) {
            throw std::bad_variant_access();
        }
    }

    template<std::size_t I>
    void validate_index() const {
        if (valueless_by_exception()) {
            throw std::bad_variant_access();
        }
        if (_index != I) {
            throw std::bad_variant_access();
        }
    }


public:
    inline static constexpr std::size_t npos = -1;

    bool valueless_by_exception() const noexcept {
        return _index == npos;
    }

    size_t index() const noexcept {
        return _index;
    }

    template<class Type, class... Types >
        requires detail::_Is_type_present<Type, Types...>
    bool holds_alternative(const std::variant<Types...>& v) noexcept {
        return detail::_Get_index_v<Type, Types...> == _index;
    }


    Variant() 
        noexcept(std::is_nothrow_default_constructible_v<
        detail::_Get_first_t<Types...>>)
        : _index(0), _storage() {
        using First_type = detail::_Get_first_t<Types...>;
        static_assert(std::is_default_constructible_v<
            detail::_Get_first_t<Types...>>,
            "First type hasn't default constructor");
        _storage.create<First_type>();
    }

    Variant(const Variant& other)
        noexcept((std::is_nothrow_copy_constructible_v<Types> && ...)) 
        : _index(other._index), _storage() {
        static_assert((std::is_copy_constructible_v<Types> && ...),
            "Someone of types haven't copy constructible"); 

        ((detail::_Get_index_v<Types, Types...> == _index ?
            _storage.create<Types>(
                other._storage.get<Types>()) 
                : void()), ...);
    }

    Variant(Variant&& other) 
        noexcept((std::is_nothrow_move_constructible_v<Types> && ...))
        : _index(other._index), _storage() {

        static_assert((std::is_move_constructible_v<Types> && ...),
            "Someone of types haven't move constructible");

        ((detail::_Get_index_v<Types, Types...> == _index ?
            _storage.create<Types>(
                std::move(other._storage.get<Types>())) 
                : void()), ...);
    }

    template<typename Type>
        requires detail::_Is_type_present<std::remove_cvref_t<Type>, Types...>
    Variant(Type&& value)
        noexcept(std::is_nothrow_constructible_v<Type, Type&&>)
        : _index(detail::_Get_index_v<std::remove_cvref_t<Type>, Types...>), _storage() {
        using Pure_type = std::remove_cvref_t<Type>;
        static_assert(std::is_constructible_v<Pure_type, Type&&>,
            "Type can't created by Type&&");

        _storage.create<Pure_type>(std::forward<Pure_type>(value));
    }

    template<typename Type, typename... Args>
        requires detail::_Is_type_present<Type, Types...>
    explicit Variant(std::in_place_type_t<Type>, Args&&... args) 
        : _index(detail::_Get_index_v<Type, Types...>), _storage() {
        static_assert(std::is_constructible_v<Type, Args...>,
            "Type can't created by these args");

        _storage.create<Type>(std::forward<Args>(args)...);
    }

    template<typename Type, typename UType, typename... Args>
        requires detail::_Is_type_present<Type, Types...>
    explicit Variant(std::in_place_type_t<Type>, std::initializer_list<UType> il, Args&&... args)
        : _index(detail::_Get_index_v<Type, Types...>), _storage() {
        static_assert(std::is_constructible_v<Type, std::initializer_list<UType>, Args...>,
            "Type can't created by these args");

        _storage.create<Type>(il, std::forward<Args>(args)...);
    }

    template<size_t I, typename... Args>
        requires detail::_Is_type_present<detail::_Get_type_t<I, Types...>, Types...>
    explicit Variant(std::in_place_index_t<I>, Args&&... args)
        : _index(I), _storage() {
        using Type = detail::_Get_type_t<I, Types...>;
        static_assert(std::is_constructible_v<Type, Args...>,
            "Type can't created by these args");

        _storage.create<Type>(std::forward<Args>(args)...);
    }

    template<size_t I, typename UType, typename... Args>
        requires detail::_Is_type_present<detail::_Get_type_t<I, Types...>, Types...>
    explicit Variant(std::in_place_index_t<I>, std::initializer_list<UType> il, Args&&... args)
        : _index(I), _storage() {
        using Type = detail::_Get_type_t<I, Types...>;
        static_assert(std::is_constructible_v<Type, std::initializer_list<UType>, Args...>,
            "Type can't created by these args");

        _storage.create<Type>(il, std::forward<Args>(args)...);
    }

    ~Variant() {
        ((detail::_Get_index_v<Types, Types...> == _index 
            ? _storage.destroy<Types>() : void()), ...);
    }


    template<typename Type, typename... Args>
        requires detail::_Is_type_present<Type, Types...>
    Type& emplace(Args&&... args) {
        static_assert(std::is_constructible_v<Type, Args...>, "Type can't be constructed from args");

        ((detail::_Get_index_v<Types, Types...> == _index ? 
            _storage.destroy<Types>() : void()), ...);

        _index = detail::_Get_index_v<Type, Types...>;

        try {
            _storage.create<Type>(std::forward<Args>(args)...);
        }
        catch (...) {
            _index = npos;
            throw;
        }
        return _storage.get<Type>();
    }

    template<typename Type, typename U, typename... Args>
        requires detail::_Is_type_present<Type, Types...>
    Type& emplace(std::initializer_list<U> il, Args&&... args) {
        static_assert(std::is_constructible_v<Type, std::initializer_list<U>, Args...>,
            "Type can't be constructed from initializer_list and args");

        ((detail::_Get_index_v<Types, Types...> == _index ? 
            _storage.destroy<Types>() : void()), ...);

        _index = detail::_Get_index_v<Type, Types...>;

        try {
            _storage.create<Type>(il, std::forward<Args>(args)...);
        }
        catch (...) {
            _index = npos;
            throw;
        }
        
        return _storage.get<Type>();
    }

    template<std::size_t I, typename... Args>
        requires (I < sizeof...(Types))
    auto& emplace(Args&&... args) {
        using Type = detail::_Get_type_t<I, Types...>;
        static_assert(std::is_constructible_v<Type, Args...>, "Type can't be constructed from args");

        ((detail::_Get_index_v<Types, Types...> == _index ?
            _storage.destroy<Types>() : void()), ...);

        _index = I;

        try {
            _storage.create<Type>(std::forward<Args>(args)...);
        }
        catch (...) {
            _index = npos;
            throw;
        }

        return _storage.get<Type>();
    }

    template<std::size_t I, typename U, typename... Args>
        requires (I < sizeof...(Types))
    auto& emplace(std::initializer_list<U> il, Args&&... args) {
        using Type = detail::_Get_type_t<I, Types...>;
        static_assert(std::is_constructible_v<Type, std::initializer_list<U>, Args...>,
            "Type can't be constructed from initializer_list and args");

        ((detail::_Get_index_v<Types, Types...> == _index ? 
            _storage.destroy<Types>() : void()), ...);

        _index = I;

        try {
            _storage.create<Type>(il, std::forward<Args>(args)...);
        }
        catch (...) {
            _index = npos;
            throw;
        }

        return _storage.get<Type>();
    }

    void swap(Variant& other)
        noexcept(((std::is_nothrow_move_constructible_v<Types> &&
                   std::is_nothrow_swappable_v<Types>) && ...)) {
    
        static_assert((std::is_move_constructible_v<Types> && ...),
            "All types in Variant must be move constructible");
        static_assert((std::is_swappable_v<Types> && ...),
            "All types in Variant must be swappable");

        if (valueless_by_exception() && other.valueless_by_exception()) {
            return;
        }

        if (_index == other._index) {
            ((detail::_Get_index_v<Types, Types...> == _index ?
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

    template<size_t isNoexcept, typename Type>
    void variant_assign(bool isSameType, size_t otherIndex, Type&& src) noexcept(isNoexcept) {
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
            ((detail::_Get_index_v<Types, Types...> == _index ?
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

    Variant& operator=(const Variant& other) 
        noexcept(((std::is_nothrow_copy_constructible_v<Types>&&
            std::is_nothrow_copy_assignable_v<Types>) && ...)) {
        if (*this == other) return *this;

        static_assert((std::is_copy_constructible_v<Types> && ...),
            "All types in Variant must be copy constructible");

        if (other.valueless_by_exception()) {   
            ((detail::_Get_index_v<Types, Types...> == _index ?
                _storage.destroy<Types>()
                : void()), ...);
            _index = npos;
            return *this;
        }

        constexpr bool isNoexcept = ((std::is_nothrow_copy_constructible_v<Types> &&
            std::is_nothrow_copy_assignable_v<Types>) && ...);

        bool isSameType = (((detail::_Get_index_v<Types, Types...> == other._index ?
            std::is_copy_assignable_v<Types> : 0) + ...) && _index == other._index);

        ((detail::_Get_index_v<Types, Types...> == other._index ?
            variant_assign<isNoexcept, const Types&>(
                isSameType, other._index, other._storage.get<Types>())
            : void()), ...);
        
        return *this;
    }


    Variant& operator=(Variant&& other) 
        noexcept(((std::is_nothrow_move_constructible_v<Types>&& 
            std::is_nothrow_move_assignable_v<Types>) && ...)) {
        if (*this == other) return *this;

        static_assert((std::is_move_constructible_v<Types> && ...),
            "All types in Variant must be move constructible");

        if (other.valueless_by_exception()) {
            ((detail::_Get_index_v<Types, Types...> == _index ?
                _storage.destroy<Types>()
                : void()), ...);
            _index = npos;
            return *this;
        }

        constexpr bool isNoexcept = ((std::is_nothrow_move_constructible_v<Types> &&
            std::is_nothrow_move_assignable_v<Types>) && ...);

        bool isSameType = (((detail::_Get_index_v<Types, Types...> == other._index ?
            std::is_move_assignable_v<Types> : 0) + ...) && _index == other._index);

        ((detail::_Get_index_v<Types, Types...> == other._index ?
            variant_assign<isNoexcept, Types&&>(
                isSameType, other._index, std::move(other._storage.get<Types>()))
            : void()), ...);

        return *this;
    }

    template<typename Type>
        requires detail::_Is_type_present<std::remove_cvref_t<Type>, Types...>
    Variant& operator=(Type&& obj)
        noexcept((std::is_nothrow_constructible_v<std::remove_cvref_t<Type>, Type&&>&&
            std::is_nothrow_assignable_v<std::remove_cvref_t<Type>, Type&&>) ||
            std::is_trivial_v<std::remove_cvref_t<Type>>) {

        using Pure_type = std::remove_cvref_t<Type>;
        static_assert(std::is_constructible_v<Pure_type, Type&&>,
            "Type must be constructible by Type&&");

        constexpr bool isNoexcept = (std::is_nothrow_constructible_v<Pure_type, Type&&> &&
            std::is_nothrow_assignable_v<Pure_type, Type&&>) ||
            std::is_trivial_v<Pure_type>;

        constexpr size_t obj_index = detail::_Get_index_v<Pure_type, Types...>;
        bool isSameType = (_index == obj_index) && std::is_assignable_v<Pure_type, Type&&>;

        variant_assign<isNoexcept, Type&&>(isSameType, obj_index, std::forward<Type>(obj));

        return *this;
    }

    bool operator==(const Variant& other) const {
        static_assert((std::equality_comparable<Types> && ...),
            "All types in Variant must support operator==");

        if (valueless_by_exception() && other.valueless_by_exception()) {
            return true;
        }

        if (valueless_by_exception() != other.valueless_by_exception()) {
            return false;
        }

        if (_index != other._index) {
            return false;
        }

        return (((detail::_Get_index_v<Types, Types...> == _index) ?
            (_storage.get<Types>() == other.get<Types>())
            : false) || ...);
    }

    bool operator!=(const Variant& other) {
        return !(*this == other);
    }

    template <typename Type>
        requires detail::_Is_type_present<Type, Types...>
    const Type& get() const& {
        validate_access<Type>();
        return _storage.get<Type>();
    }

    template <typename Type>
        requires detail::_Is_type_present<Type, Types...>
    Type& get()& {
        validate_access<Type>();
        return _storage.get<Type>();
    }

    template <typename Type>
        requires detail::_Is_type_present<Type, Types...>
    const Type&& get() const&& {
        validate_access<Type>();
        return std::move(_storage.get<Type>());
    }

    template <typename Type>
        requires detail::_Is_type_present<Type, Types...>
    Type&& get()&& {
        validate_access<Type>();
        return std::move(_storage.get<Type>());
    }

    template <size_t I>
        requires detail::_Is_index_of_alternative<I, Types...>
    const detail::_Get_type_t<I, Types...>& get() const& {
        validate_index<I>();
        return _storage.get<detail::_Get_type_t<I, Types...>>();
    }

    template <size_t I>
        requires detail::_Is_index_of_alternative<I, Types...>
    detail::_Get_type_t<I, Types...>& get()& {
        validate_index<I>();
        return _storage.get<detail::_Get_type_t<I, Types...>>();
    }

    template <size_t I>
        requires detail::_Is_index_of_alternative<I, Types...>
    const detail::_Get_type_t<I, Types...>&& get() const&& {
        validate_index<I>();    
        return std::move(_storage.get<detail::_Get_type_t<I, Types...>>());
    }

    template <size_t I>
        requires detail::_Is_index_of_alternative<I, Types...>
    detail::_Get_type_t<I, Types...>&& get()&& {
        validate_index<I>();
        return std::move(_storage.get<detail::_Get_type_t<I, Types...>>());
    }

    template <typename Type>
        requires detail::_Is_type_present<Type, Types...>
    Type* get_if() & noexcept {
        return (!valueless_by_exception() &&
            _index == detail::_Get_index_v<Type, Types...>)
            ? &_storage.get<Type>()
            : nullptr;
    }

    template <typename Type>
        requires detail::_Is_type_present<Type, Types...>
    const Type* get_if() const& noexcept {
        return (!valueless_by_exception() &&
            _index == detail::_Get_index_v<Type, Types...>)
            ? &_storage.get<Type>()
            : nullptr;
    }

    template <std::size_t I>
        requires detail::_Is_index_of_alternative<I, Types...>
    detail::_Get_type_t<I, Types...>* get_if() & noexcept {
        return (!valueless_by_exception() &&
            _index == I)
            ? &_storage.get<detail::_Get_type_t<I, Types...>>()
            : nullptr;
    }

    template <std::size_t I>
        requires detail::_Is_index_of_alternative<I, Types...>
    const detail::_Get_type_t<I, Types...>* get_if() const& noexcept {
        return (!valueless_by_exception() &&
            _index == I)
            ? &_storage.get<detail::_Get_type_t<I, Types...>>()
            : nullptr;
    }
};
