#include "Variant.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <variant>

struct Point {
    int _x;
    int _y;

    Point(int x, int y) : _x(x), _y(y) {}
};

int main() {
    Point p(1, 1);
    Variant<int, double> v;
    Variant<int, double> v1(1);
    Variant<int, double> v2(1.0);
    Variant<int, double> v3(v);
    Variant<int, double> v4(std::move(v));
    Variant<int, double> v5(Variant<int, double>(1));
    v1 = 1.0;
    v2 = 2;
    v3 = v2;
    v4 = std::move(v3);
    v5 = Variant<int, double>(1);
    v5 = v4.get<int>();
    v5 = std::move(v4.get<int>());
    std::cout << v5.get<int>() << std::endl;
    std::cout << v4.get<int>() << std::endl;
    Variant<int, std::string> v6 = std::string("123");
    Variant<int, std::string> v7 = std::move(v6.get<std::string>());
    auto* ptr = v7.get_if<std::string>();
    std::cout << *ptr << std::endl;

    Variant<int, std::string> v01;

    v01.emplace<0>(42);
    std::cout << v01.get<0>() << std::endl; 

    v01.emplace<1>("hello");
    std::cout << v01.get<1>() << std::endl;

    {
        Variant<int, std::string> v02;

        v02.emplace<std::string>("world");
        std::cout << v02.get<std::string>() << std::endl;
    }
    Variant<std::vector<int>, std::string> v03(std::string(""));

    std::allocator<int> alloc;
    v03.emplace<std::vector<int>>({ 1, 2, 3, 4 }, alloc);

    for (int x : v03.get<std::vector<int>>()) {
        std::cout << x << ' ';
    }
    std::cout << '\n';

    return 0;
}
