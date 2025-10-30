#include "Variant.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <variant>


int main() {
    // 1. Прямая инициализация
    constexpr Variant<int, double> v1(42);

    // 2. Uniform initialization
    constexpr Variant<int, double> v2{ 42 };

    // 3. Копирующая инициализация (вызывает конструктор копирования)
    constexpr Variant<int, double> v3 = Variant<int, double>{ 42 };

    // 4. List initialization
    constexpr Variant<int, double> v4 = { 42 };

    // 5. Конструктор с in_place_type
    constexpr Variant<int, double> v5{ std::in_place_type<int>, 42 };


    return 0;
}
