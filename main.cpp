#include <iostream>
#include <list>
#include <vector>

#include "meta_operation.hpp"

#include <benchmark/benchmark.h>
#include <random>

namespace dim {
    constexpr size_t dim1 = 400;
    constexpr size_t dim2 = 400;
    constexpr size_t dim3 = 400;
}

template<typename ValueType>
std::vector<std::vector<std::vector<ValueType>>> random_generate_vector();

template<>
std::vector<std::vector<std::vector<double>>> random_generate_vector() {
    using namespace dim;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    std::uniform_int_distribution<> dim_dis(1, 100); // 随机生成1到10的整数

    std::vector<std::vector<std::vector<double>>> arr(
        dim1, std::vector<std::vector<double>>(dim2, std::vector<double>(dim3)));
    for (auto&i: arr) {
        for (auto&j: i) {
            for (auto&k: j) {
                k = dis(gen);
            }
        }
    }

    // benchmark::DoNotOptimize(arr);

    return arr;
}

template<>
std::vector<std::vector<std::vector<std::string>>> random_generate_vector() {
    using namespace dim;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    std::uniform_int_distribution<> dim_dis(1, 100); // 随机生成1到10的整数
    std::uniform_int_distribution<> char_dim(1, 127);

    std::vector<std::vector<std::vector<std::string>>> arr(
        dim1, std::vector<std::vector<std::string>>(dim2, std::vector<std::string>(dim3)));
    for (auto&i: arr) {
        for (auto&j: i) {
            for (auto&k: j) {
                k = std::string(dim_dis(gen), char_dim(gen));
            }
        }
    }

    // benchmark::DoNotOptimize(arr);

    return arr;
}


void BM_create_vector_and_resize_double(benchmark::State&state) {
    using namespace dim;
    auto arr = random_generate_vector<double>();

    for (auto const&_: state) {
        std::vector<std::vector<std::vector<double>>> dest(
            dim1, std::vector<std::vector<double>>(dim2, std::vector<double>(dim3)));
        // benchmark::DoNotOptimize(dest);
    }
}

BENCHMARK(BM_create_vector_and_resize_double);

void BM_create_vector_and_resize_std_string(benchmark::State&state) {
    using namespace dim;
    auto arr = random_generate_vector<std::string>();

    for (auto const&_: state) {
        std::vector<std::vector<std::vector<std::string>>> dest(
            dim1, std::vector<std::vector<std::string>>(dim2, std::vector<std::string>(dim3)));
        // benchmark::DoNotOptimize(dest);
    }
}

BENCHMARK(BM_create_vector_and_resize_std_string);

void BM_flat_transform(benchmark::State&state) {
    using namespace dim;
    auto arr = random_generate_vector<double>();

    for (auto const&_: state) {
        // state.PauseTiming();
        std::vector<std::vector<std::vector<double>>> dest(
            dim1, std::vector<std::vector<double>>(dim2, std::vector<double>(dim3)));
        // state.ResumeTiming();

        meta_operation::flat_transform(arr.begin(), arr.end(), dest.begin(), [](const double a) {
            return 1 + a;
        });
    }
}

BENCHMARK(BM_flat_transform);

void BM_std_transform_double(benchmark::State&state) {
    using namespace dim;
    auto arr = random_generate_vector<double>();

    for (auto const&_: state) {
        // state.PauseTiming();
        std::vector<std::vector<std::vector<double>>> dest(
            dim1, std::vector<std::vector<double>>(dim2, std::vector<double>(dim3)));
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

BENCHMARK(BM_std_transform_double);

void BM_std_transform_string(benchmark::State&state) {
    using namespace dim;
    auto arr = random_generate_vector<std::string>();

    for (auto const&_: state) {
        // state.PauseTiming();
        std::vector<std::vector<std::vector<std::string>>> dest(
            dim1, std::vector<std::vector<std::string>>(dim2, std::vector<std::string>(dim3)));
        // state.ResumeTiming();

        for (size_t i = 0; i < dim1; ++i) {
            for (size_t j = 0; j < dim2; ++j) {
                std::transform(arr[i][j].begin(), arr[i][j].end(), dest[i][j].begin(), [](std::string&a) {
                    std::reverse(a.begin(), a.end());
                    return a;
                });
            }
        }
    }
}

BENCHMARK(BM_std_transform_string);

void BM_flat_transform_with_pre_resize_double(benchmark::State&state) {
    using namespace dim;
    auto arr = random_generate_vector<double>();

    for (auto const&_: state) {
        // state.PauseTiming();
        std::vector<std::vector<std::vector<double>>> dest(
            dim1, std::vector<std::vector<double>>(dim2, std::vector<double>(dim3)));
        // state.ResumeTiming();

        meta_operation::flat_transform(arr, dest, [](const double a) {
            return 1 + a;
        });
    }
}

BENCHMARK(BM_flat_transform_with_pre_resize_double);

void BM_flat_transform_with_pre_resize_string(benchmark::State&state) {
    using namespace dim;
    auto arr = random_generate_vector<std::string>();

    for (auto const&_: state) {
        // state.PauseTiming();
        std::vector<std::vector<std::vector<std::string>>> dest(
            dim1, std::vector<std::vector<std::string>>(dim2, std::vector<std::string>(dim3)));
        // state.ResumeTiming();

        meta_operation::flat_transform(arr, dest, [](std::string& a) {
            std::reverse(a.begin(), a.end());
            return a;
        });
    }
}

BENCHMARK(BM_flat_transform_with_pre_resize_string);

void BM_flat_transform_with_back_insert_iterator(benchmark::State&state) {
    using namespace dim;
    auto arr = random_generate_vector<double>();

    for (auto const&_: state) {
        std::vector<std::vector<std::vector<double>>> dest;
        dest.reserve(arr.size());
        meta_operation::flat_transform(arr.begin(), arr.end(), std::back_inserter(dest), [](const double&a) {
            return 1 + a;
        });
    }
}

BENCHMARK(BM_flat_transform_with_back_insert_iterator);

void BM_flat_transform_with_back_insert_iterator_string(benchmark::State&state) {
    using namespace dim;
    auto arr = random_generate_vector<std::string>();

    for (auto const&_: state) {
        std::vector<std::vector<std::vector<std::string>>> dest;
        dest.reserve(arr.size());
        meta_operation::flat_transform(arr.begin(), arr.end(), std::back_inserter(dest), [](std::string&a) {
            std::reverse(a.begin(), a.end());
            return a;
        });
    }
}

BENCHMARK(BM_flat_transform_with_back_insert_iterator_string);

void BM_std_transform_with_back_insert_iterator(benchmark::State&state) {
    using namespace dim;
    auto arr = random_generate_vector<double>();

    for (auto const&_: state) {
        std::vector<std::vector<std::vector<double>>> dest;
        dest.reserve(arr.size());
        std::transform(arr.begin(), arr.end(), std::back_inserter(dest), [](std::vector<std::vector<double>>&a) {
            std::vector<std::vector<double>> destSubVector(a.size());
            // destSubVector.reserve(a.size());
            std::transform(a.begin(), a.end(), destSubVector.begin(), [](std::vector<double>&a) {
                std::vector<double> destSubVectorSubVector(a.size());
                // destSubVectorSubVector.reserve(a.size());
                std::transform(a.begin(), a.end(), destSubVectorSubVector.begin(), [](double&a) {
                    return a + 1;
                });
                return destSubVectorSubVector;
            });
            return destSubVector;
        });
    }
}

BENCHMARK(BM_std_transform_with_back_insert_iterator);

void BM_std_transform_with_back_insert_iterator_string(benchmark::State&state) {
    using namespace dim;
    auto arr = random_generate_vector<std::string>();

    for (auto const&_: state) {
        std::vector<std::vector<std::vector<std::string>>> dest;
        dest.reserve(arr.size());
        std::transform(arr.begin(), arr.end(), std::back_inserter(dest), [](std::vector<std::vector<std::string>>&a) {
            std::vector<std::vector<std::string>> destSubVector(a.size());
            // destSubVector.reserve(a.size());
            std::transform(a.begin(), a.end(), destSubVector.begin(), [](std::vector<std::string>&a) {
                std::vector<std::string> destSubVectorSubVector(a.size());
                // destSubVectorSubVector.reserve(a.size());
                std::transform(a.begin(), a.end(), destSubVectorSubVector.begin(), [](std::string&a) {
                    std::reverse(a.begin(), a.end());
                    return a;
                });
                return destSubVectorSubVector;
            });
            return destSubVector;
        });
    }
}

BENCHMARK(BM_std_transform_with_back_insert_iterator_string);

BENCHMARK_MAIN();
