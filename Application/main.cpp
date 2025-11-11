#include <iostream>
#include <string>
#include "Variant.hpp"


int main() {
    constexpr std::string_view text = 
        "Thank you, Herb Sutter, for the 20th standard!";
    constexpr Variant<std::string_view, bool> v(text);
    constexpr Variant<size_t> v1((size_t)46);
    if constexpr (v.get<0>().size() == v1.get<0>()) {
        std::cout << v.get<0>() << std::endl;
    }

    return 0;
}
