#include <iostream>
#include <list>
#include <vector>

#include "meta_operation.hpp"

#include <benchmark/benchmark.h>
#include <random>

namespace dim {
    constexpr size_t dim1 = 400;
    constexpr size_t dim2 = 100;
    constexpr size_t dim3 = 100;
}


std::vector<std::vector<std::vector<double> > > random_generate_vector() {
    using namespace dim;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    std::uniform_int_distribution<> dim_dis(1, 10); // 随机生成1到10的整数

    std::vector<std::vector<std::vector<double> > > arr(
        dim1, std::vector<std::vector<double> >(dim2, std::vector<double>(dim3)));
    for (auto &i: arr) {
        for (auto &j: i) {
            for (auto &k: j) {
                k = dis(gen);
            }
        }
    }

    // benchmark::DoNotOptimize(arr);

    return arr;
}

void BM_create_vector_and_resize(benchmark::State &state) {
    using namespace dim;
    auto arr = random_generate_vector();

    for (auto const &_: state) {
        std::vector<std::vector<std::vector<double> > > dest(
            dim1, std::vector<std::vector<double> >(dim2, std::vector<double>(dim3)));
        benchmark::DoNotOptimize(dest);
    }
}

BENCHMARK(BM_create_vector_and_resize);

void BM_flat_transform(benchmark::State &state) {
    using namespace dim;
    auto arr = random_generate_vector();

    for (auto const &_: state) {
        // state.PauseTiming();
        std::vector<std::vector<std::vector<double> > > dest(
            dim1, std::vector<std::vector<double> >(dim2, std::vector<double>(dim3)));
        // state.ResumeTiming();

        meta_operation::flat_transform(arr.begin(), arr.end(), dest.begin(), [](const double a) {
            return 1 + a;
        });
    }
}

BENCHMARK(BM_flat_transform);

void BM_std_transform(benchmark::State &state) {
    using namespace dim;
    auto arr = random_generate_vector();

    for (auto const &_: state) {
        // state.PauseTiming();
        std::vector<std::vector<std::vector<double> > > dest(
            dim1, std::vector<std::vector<double> >(dim2, std::vector<double>(dim3)));
        // state.ResumeTiming();

        for (size_t i = 0; i < dim1; ++i) {
            for (size_t j = 0; j < dim2; ++j) {
                std::transform(arr[i][j].begin(), arr[i][j].end(), dest[i][j].begin(), [](const double a) {
                    return a + 1;
                });
            }
        }
    }
}

BENCHMARK(BM_std_transform);

void BM_flat_transform_with_pre_resize(benchmark::State &state) {
    using namespace dim;
    auto arr = random_generate_vector();

    for (auto const &_: state) {
        // state.PauseTiming();
        std::vector<std::vector<std::vector<double> > > dest(
            dim1, std::vector<std::vector<double> >(dim2, std::vector<double>(dim3)));
        // state.ResumeTiming();

        meta_operation::flat_transform(arr, dest, [](const double a) {
            return 1 + a;
        });
    }
}

BENCHMARK(BM_flat_transform_with_pre_resize);

void BM_flat_transform_with_back_insert_iterator(benchmark::State &state) {
    using namespace dim;
    auto arr = random_generate_vector();

    for (auto const &_: state) {
        std::vector<std::vector<std::vector<double> > > dest;
        meta_operation::flat_transform(arr.begin(), arr.end(), std::back_inserter(dest), [](const double a) {
            return 1 + a;
        });
    }
}

BENCHMARK(BM_flat_transform_with_back_insert_iterator);

void BM_std_transform_with_back_insert_iterator(benchmark::State &state) {
    using namespace dim;
    auto arr = random_generate_vector();

    for (auto const &_: state) {
        std::vector<std::vector<std::vector<double> > > dest;
        std::transform(arr.begin(), arr.end(), std::back_inserter(dest), [](std::vector<std::vector<double> > &a) {
            std::vector<std::vector<double> > destSubVector(a.size());
            std::transform(a.begin(), a.end(), destSubVector.begin(), [](std::vector<double> &a) {
                std::vector<double> destSubVectorSubVector(a.size());
                std::transform(a.begin(), a.end(), destSubVectorSubVector.begin(), [](double &a) {
                    return a + 1;
                });
                return destSubVectorSubVector;
            });
            return destSubVector;
        });
    }
}

BENCHMARK(BM_std_transform_with_back_insert_iterator);

BENCHMARK_MAIN();

// int main() {
//     std::vector<int> ints = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
//
//     // std::cout << (ints | views::for_each([&](int i) {
//     //     return yield_if(i % 2 == 0, i);
//     // }));
//     //
//     // std::cout << (ints | views::enumerate | views::transform([](auto const&item) {
//     //     auto [index,value] = item;
//     //     return index;
//     // }));
//
//     const std::vector<std::vector<int> > b{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, {1, 2, 3, 4, 5}};
//
//     meta_operation::replace_other_container_type_with<std::vector<std::vector<std::vector<int> > >, std::list>::type a;
//     // other_container_type_helper<std::vector<std::vector<std::vector<int>>>, std::list> a;
//     // meta_operation::flat_foreach(b, [](int const&i) {
//     //     std::cout << i;
//     // });
//     meta_operation::flat_foreach(b.begin(), b.end(), [](int const &i) {
//         std::cout << i;
//     });
//     std::cout << typeid(*b.begin()).name();
//     std::vector<std::vector<std::vector<int> > > o;
//     std::vector<std::vector<std::vector<int> > > &p = o;
//
//     auto d = meta_operation::flat_transform(b, [](int const &item) {
//         return item + 1;
//     });
//
//     meta_operation::flat_foreach(d, [](int const &i) {
//         std::cout << i;
//     });
//
//     std::vector<std::vector<int> > dest(2);
//     meta_operation::flat_transform(b.begin(), b.end(), std::back_inserter(dest), [](int a) {
//         return 1 + a;
//     });
//
//     // auto kkk = is_specialization_of<std::vector<int>, std::list>::value;
//     return 0;
// }
