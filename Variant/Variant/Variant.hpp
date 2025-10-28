#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <variant>
#include <iostream>
#include "../Detail/Detail.hpp"

namespace my {

}
template <typename... Types>
class Variant final {
private:
    class Storage final {
    private:
        static constexpr size_t _max_size = std::max({ sizeof(Types)... });
        alignas(std::max({ alignof(Types)... })) char _buffer[_max_size];
    public:
        template<typename Type, typename... Args>
        void create(Args&&... args) {
            new (_buffer) Type(std::forward<Args>(args)...);
        }

        template<typename Type>
        void destroy() {
            reinterpret_cast<Type*>(_buffer)->~Type();
        }

        template<typename Type>
        const Type& get() const {
            return *(reinterpret_cast<const Type*>(_buffer));
        }

        template<typename Type>
        Type& get() {
            return *(reinterpret_cast<Type*>(_buffer));
        }

    };

    static constexpr size_t _count = sizeof...(Types);
    size_t _index;
    Storage _storage;

    template<typename Type>
    void validate_access() const {
        static_assert(detail::_Are_unique_v<Types...>, "Variant have sames types");
        if (valueless_by_exception()) {
            throw std::bad_variant_access();
        }
        if (_index != detail::_Get_index_v<Type, Types...>) {
            throw std::bad_variant_access();
        }
    }

    template<std::size_t I>
    void validate_index() const {
        static_assert(detail::_Are_unique_v<Types...>, "Variant have sames types with given index");
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
        noexcept((std::is_move_constructible_v<Types> && ...))
        : _index(other._index), _storage() {

        static_assert((std::is_move_constructible_v<Types> && ...),
            "Someone of types haven't move constructible");

        ((detail::_Get_index_v<Types, Types...> == _index ?
            _storage.create<Types>(
                std::move(other._storage.get<Types>())) 
                : void()), ...);
    }

    template<typename Type>
        requires detail::_Is_type_present<Type, Types...>
    Variant(Type&& value)
        noexcept(std::is_nothrow_constructible_v<Type, Type&&>)
        : _index(detail::_Get_index_v<Type, Types...>), _storage() {

        static_assert(std::is_constructible_v<Type, Type&&>,
            "Type can't created by Type&&");

        _storage.create<Type>(std::forward<Type>(value));
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
        static_assert(detail::_Are_unique_v<Types...>, "Variant has duplicate types");
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
        static_assert(detail::_Are_unique_v<Types...>, "Variant has duplicate types");
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


    Variant& operator=(const Variant& other) {
        if (this == &other) return *this;

        ((detail::_Get_index_v<Types, Types...> == _index 
            ? _storage.destroy<Types>()
            : void()), ...);
        _index = other._index;

        try {
            ((detail::_Get_index_v <Types, Types... > == _index ?
                _storage.create<Types>(other._storage.get<Types>())
                : void()), ...);
        }
        catch (...) {
            _index = npos;
            throw;
        }


        return *this;
    }

    Variant& operator=(Variant&& other)
        noexcept(((std::is_nothrow_move_constructible_v<Types>&&
            std::is_nothrow_move_assignable_v<Types>) && ...)) {
        if (this == &other) return *this;

        static_assert(((std::is_move_constructible_v<Types> &&
            std::is_move_assignable_v<Types>) && ...),
            "All types must have move construct");

        ((detail::_Get_index_v<Types, Types...> == _index 
            ? _storage.destroy<Types>()
            : void()), ...);
        _index = other._index;

        if constexpr (((std::is_nothrow_move_constructible_v<Types> &&
            std::is_nothrow_move_assignable_v<Types>) && ...)) {
            ((detail::_Get_index_v<Types, Types...> == _index ?
                _storage.create<Types>(
                    std::move(other._storage.get<Types>()))
                : void()), ...);
        }
        else {
            try {
                ((detail::_Get_index_v<Types, Types...> == _index ?
                    _storage.create<Types>(
                        std::move(other._storage.get<Types>()))
                    : void()), ...);
            }
            catch (...) {
                _index = npos;
                throw;
            }
        }

        return *this;
    }


    template<typename Type>
        requires detail::_Is_type_present<std::remove_cvref_t<Type>, Types...>
    Variant& operator=(Type&& obj) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Type>, Type&&>) {
        using Pure_type = std::remove_cvref_t<Type>;
        static_assert(std::is_constructible_v<Pure_type, Type&&>,
            "Type must be constructible by Type&&");

        ((detail::_Get_index_v<Types, Types...> == _index ? _storage.destroy<Types>()
            : void()), ...);
        _index = detail::_Get_index_v<Pure_type, Types...>;

        if constexpr (std::is_nothrow_constructible_v<Pure_type, Type&&>) {
            _storage.create<Pure_type>(std::forward<Type>(obj));
        }
        else {
            try {

                _storage.create<Pure_type>(std::forward<Type>(obj));
            }
            catch (...) {
                _index = npos;
                throw;
            }
        }

        return *this;
    }

    bool operator==(const Variant& other) const {
        static_assert((requires { 
            std::declval<Types>() == std::declval<Types>(); 
        } && ...),
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

    template<typename Type>
        requires detail::_Is_type_present<Type, Types...>
    const Type& get() const& {
        validate_access<Type>();
        return _storage.get<Type>();
    }
    
    template<typename Type>
        requires detail::_Is_type_present<Type, Types...>
    Type& get() & {
        validate_access<Type>();
        return _storage.get<Type>();
    }

    template<typename Type>
        requires detail::_Is_type_present<Type, Types...>
    const Type&& get() const && {
        validate_access<Type>();
        return std::move(_storage.get<Type>());
    }

    template<typename Type>
        requires detail::_Is_type_present<Type, Types...>
    Type&& get() && {
        validate_access<Type>();
        return std::move(_storage.get<Type>());
    }

    template<size_t I>
        requires detail::_Is_index_of_alternative<I, Types...>
    const detail::_Get_type_t<I, Types...>& get() const& {
        validate_index<I>();
        return _storage.get<detail::_Get_type_t<I, Types...>>();
    }

    template<size_t I>
        requires detail::_Is_index_of_alternative<I, Types...>
    detail::_Get_type_t<I, Types...>& get() & {
        validate_index<I>();
        return _storage.get<detail::_Get_type_t<I, Types...>>();
    }

    template<size_t I>
        requires detail::_Is_index_of_alternative<I, Types...>
    const detail::_Get_type_t<I, Types...>&& get() const&& {
        validate_index<I>();
        return std::move(_storage.get<detail::_Get_type_t<I, Types...>>());
    }

    template<size_t I>
        requires detail::_Is_index_of_alternative<I, Types...>
    detail::_Get_type_t<I, Types...>&& get() && {
        validate_index<I>();
        return std::move(_storage.get<detail::_Get_type_t<I, Types...>>());
    }

    template<typename Type>
        requires detail::_Is_type_present<Type, Types...>
    Type* get_if() & noexcept {
        static_assert(detail::_Are_unique_v<Types...>, "Variant have sames types");
        return (!valueless_by_exception() &&
            _index == detail::_Get_index_v<Type, Types...>)
            ? &_storage.get<Type>()
            : nullptr;
    }

    template<typename Type>
        requires detail::_Is_type_present<Type, Types...>
    const Type* get_if() const& noexcept {
        static_assert(detail::_Are_unique_v<Types...>, "Variant have sames types");
        return (!valueless_by_exception() &&
            _index == detail::_Get_index_v<Type, Types...>)
            ? &_storage.get<Type>()
            : nullptr;
    }

    template<std::size_t I>
        requires detail::_Is_index_of_alternative<I, Types...>
    detail::_Get_type_t<I, Types...>* get_if() & noexcept {
        static_assert(detail::_Are_unique_v<Types...>, "Variant have sames types");
        return (!valueless_by_exception() && 
            _index == I)
            ? &_storage.get<detail::_Get_type_t<I, Types...>>()
            : nullptr;
    }

    template<std::size_t I>
        requires detail::_Is_index_of_alternative<I, Types...>
    const detail::_Get_type_t<I, Types...>* get_if() const& noexcept {
        static_assert(detail::_Are_unique_v<Types...>, "Variant have sames types");
        return (!valueless_by_exception() &&
            _index == I)
            ? &_storage.get<detail::_Get_type_t<I, Types...>>()
            : nullptr;
    }
};