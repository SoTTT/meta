#include <iostream>
#include <list>
#include <vector>

#include "meta_operation.hpp"


int main() {
    std::vector<int> ints = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // std::cout << (ints | views::for_each([&](int i) {
    //     return yield_if(i % 2 == 0, i);
    // }));
    //
    // std::cout << (ints | views::enumerate | views::transform([](auto const&item) {
    //     auto [index,value] = item;
    //     return index;
    // }));

    const std::vector<std::vector<int>> b{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, {1, 2, 3, 4, 5}};

    meta_operation::replace_other_container_type_with<std::vector<std::vector<std::vector<int>>>, std::list>::type a;
    // other_container_type_helper<std::vector<std::vector<std::vector<int>>>, std::list> a;
    // meta_operation::flat_foreach(b, [](int const&i) {
    //     std::cout << i;
    // });
    meta_operation::flat_foreach(b.begin(), b.end(), [](int const&i) {
        std::cout << i;
    });
    std::cout << typeid(*b.begin()).name();
    std::vector<std::vector<std::vector<int>>> o;
    std::vector<std::vector<std::vector<int>>>&p = o;

    auto d = meta_operation::flat_transform(b, [](int const&item) {
        return item + 1;
    });

    meta_operation::flat_foreach(d, [](int const&i) {
        std::cout << i;
    });

    std::vector<std::vector<int>> dest(2);
    meta_operation::flat_transform(b.begin(), b.end(), std::back_inserter(dest), [](int a) {
        return 1 + a;
    });

    // auto kkk = is_specialization_of<std::vector<int>, std::list>::value;
    return 0;
}
