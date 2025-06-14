#include <iostream>
#include <list>
#include <vector>

#include "md_operation.hpp"
#include "oatpp_helper.hpp"

#include <benchmark/benchmark.h>
#include <random>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(90, 100);

namespace dim {
    const size_t dim1 = dis(gen);
    const size_t dim2 = dis(gen);
    const size_t dim3 = dis(gen);
}

template<typename ValueType>
std::vector<std::vector<std::vector<ValueType> > > random_generate_vector();

template<>
std::vector<std::vector<std::vector<double> > > random_generate_vector() {
    using namespace dim;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    std::uniform_int_distribution<> dim_dis(1, 100); // 随机生成1到10的整数

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

template<>
std::vector<std::vector<std::vector<std::string> > > random_generate_vector() {
    using namespace dim;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    std::uniform_int_distribution<> dim_dis(1, 100); // 随机生成1到10的整数
    std::uniform_int_distribution<> char_dim(1, 127);

    std::vector<std::vector<std::vector<std::string> > > arr(
        dim1, std::vector<std::vector<std::string> >(dim2, std::vector<std::string>(dim3)));
    for (auto &i: arr) {
        for (auto &j: i) {
            for (auto &k: j) {
                k = std::string(dim_dis(gen), char_dim(gen));
            }
        }
    }

    // benchmark::DoNotOptimize(arr);

    return arr;
}


void BM_create_vector_and_resize_double(benchmark::State &state) {
    using namespace dim;
    auto arr = random_generate_vector<double>();

    for (auto const &_: state) {
        std::vector<std::vector<std::vector<double> > > dest(
            dim1, std::vector<std::vector<double> >(dim2, std::vector<double>(dim3)));
        // benchmark::DoNotOptimize(dest);
    }
}


void BM_create_vector_and_resize_std_string(benchmark::State &state) {
    using namespace dim;
    auto arr = random_generate_vector<std::string>();

    for (auto const &_: state) {
        std::vector<std::vector<std::vector<std::string> > > dest(
            dim1, std::vector<std::vector<std::string> >(dim2, std::vector<std::string>(dim3)));
        // benchmark::DoNotOptimize(dest);
    }
}


void BM_flat_transform(benchmark::State &state) {
    using namespace dim;
    auto arr = random_generate_vector<double>();

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


void BM_std_transform_double(benchmark::State &state) {
    using namespace dim;
    auto arr = random_generate_vector<double>();

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


void BM_std_transform_string(benchmark::State &state) {
    using namespace dim;
    auto arr = random_generate_vector<std::string>();

    for (auto const &_: state) {
        // state.PauseTiming();
        std::vector<std::vector<std::vector<std::string> > > dest(
            dim1, std::vector<std::vector<std::string> >(dim2, std::vector<std::string>(dim3)));
        // state.ResumeTiming();

        for (size_t i = 0; i < dim1; ++i) {
            for (size_t j = 0; j < dim2; ++j) {
                std::transform(arr[i][j].begin(), arr[i][j].end(), dest[i][j].begin(), [](std::string &a) {
                    std::reverse(a.begin(), a.end());
                    return a;
                });
            }
        }
    }
}


void BM_flat_transform_with_pre_resize_double(benchmark::State &state) {
    using namespace dim;
    auto arr = random_generate_vector<double>();

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


void BM_flat_transform_with_pre_resize_string(benchmark::State &state) {
    using namespace dim;
    auto arr = random_generate_vector<std::string>();

    for (auto const &_: state) {
        // state.PauseTiming();
        std::vector<std::vector<std::vector<std::string> > > dest(
            dim1, std::vector<std::vector<std::string> >(dim2, std::vector<std::string>(dim3)));
        // state.ResumeTiming();

        meta_operation::flat_transform(arr, dest, [](std::string &a) {
            std::reverse(a.begin(), a.end());
            return a;
        });
    }
}


void BM_flat_transform_with_back_insert_iterator(benchmark::State &state) {
    using namespace dim;
    auto arr = random_generate_vector<double>();

    for (auto const &_: state) {
        std::vector<std::vector<std::vector<double> > > dest;
        dest.reserve(arr.size());
        meta_operation::flat_transform(arr.begin(), arr.end(), std::back_inserter(dest), [](const double &a) {
            return 1 + a;
        });
    }
}


void BM_flat_transform_with_back_insert_iterator_string(benchmark::State &state) {
    using namespace dim;
    auto arr = random_generate_vector<std::string>();

    for (auto const &_: state) {
        std::vector<std::vector<std::vector<std::string> > > dest;
        dest.reserve(arr.size());
        meta_operation::flat_transform(arr.begin(), arr.end(), std::back_inserter(dest), [](std::string &a) {
            std::reverse(a.begin(), a.end());
            return a;
        });
    }
}


void BM_std_transform_with_back_insert_iterator(benchmark::State &state) {
    using namespace dim;
    auto arr = random_generate_vector<double>();

    for (auto const &_: state) {
        std::vector<std::vector<std::vector<double> > > dest;
        dest.reserve(arr.size());
        std::transform(arr.begin(), arr.end(), std::back_inserter(dest), [](std::vector<std::vector<double> > &a) {
            std::vector<std::vector<double> > destSubVector(a.size());
            // destSubVector.reserve(a.size());
            std::transform(a.begin(), a.end(), destSubVector.begin(), [](std::vector<double> &a) {
                std::vector<double> destSubVectorSubVector(a.size());
                // destSubVectorSubVector.reserve(a.size());
                std::transform(a.begin(), a.end(), destSubVectorSubVector.begin(), [](double &a) {
                    return a + 1;
                });
                return destSubVectorSubVector;
            });
            return destSubVector;
        });
    }
}


void BM_std_transform_with_back_insert_iterator_string(benchmark::State &state) {
    using namespace dim;
    auto arr = random_generate_vector<std::string>();

    for (auto const &_: state) {
        std::vector<std::vector<std::vector<std::string> > > dest;
        dest.reserve(arr.size());
        std::transform(arr.begin(), arr.end(), std::back_inserter(dest), [](std::vector<std::vector<std::string> > &a) {
            std::vector<std::vector<std::string> > destSubVector(a.size());
            // destSubVector.reserve(a.size());
            std::transform(a.begin(), a.end(), destSubVector.begin(), [](std::vector<std::string> &a) {
                std::vector<std::string> destSubVectorSubVector(a.size());
                // destSubVectorSubVector.reserve(a.size());
                std::transform(a.begin(), a.end(), destSubVectorSubVector.begin(), [](std::string &a) {
                    std::reverse(a.begin(), a.end());
                    return a;
                });
                return destSubVectorSubVector;
            });
            return destSubVector;
        });
    }
}


void oatppTest(benchmark::State &state) {
    using namespace meta_operation::type_traits::oatpp;

    // meta_operation::type_traits::oatpp::is_oatpp_wrapper
    static_assert(is_oatpp_wrapper<oatpp::Vector<oatpp::Float64> >::value);
    static_assert(!is_oatpp_wrapper<std::vector<double> >::value);
    static_assert(is_oatpp_wrapper<oatpp::UnorderedMap<oatpp::String, oatpp::Float64> >::value);

    // meta_operation::type_traits::oatpp::is_oatpp_container_wrapper
    static_assert(is_oatpp_container_wrapper<oatpp::Vector<double> >::value);
    static_assert(!is_oatpp_container_wrapper<oatpp::Float64>::value);
    static_assert(!is_oatpp_container_wrapper<std::vector<double> >::value);
    static_assert(!is_oatpp_container_wrapper<double>::value);
    static_assert(is_oatpp_container_wrapper<oatpp::UnorderedMap<oatpp::String, oatpp::Float64> >::value);

    // meta_operation::type_traits::oatpp::is_oatpp_primitive_wrapper
    static_assert(is_oatpp_primitive_wrapper<oatpp::Float64>::value);
    static_assert(!is_oatpp_primitive_wrapper<oatpp::Vector<double> >::value);
    static_assert(!is_oatpp_primitive_wrapper<std::vector<double> >::value);
    static_assert(!is_oatpp_primitive_wrapper<double>::value);
    static_assert(is_oatpp_primitive_wrapper<oatpp::Int64>::value);

    // meta_operation::type_traits::oatpp::is_oatpp_map_container_wrapper
    static_assert(is_oatpp_map_container_wrapper<oatpp::UnorderedMap<oatpp::String, oatpp::Float64> >::value);
    static_assert(!is_oatpp_map_container_wrapper<oatpp::Vector<oatpp::Float64> >::value);

    using namespace oatpp::data::mapping::type;
    auto value = static_cast<__class::Collection::PolymorphicDispatcher const *>
            (oatpp::Vector<Float64>::Class::getType()->polymorphicDispatcher)
            ->createObject().cast<oatpp::Vector<Float64> >();

    static_assert(std::is_same<unwrapper<double>::type, double>::value);
    static_assert(std::is_same<unwrapper<oatpp::Float64>::type, double>::value);
    static_assert(!std::is_same<unwrapper<oatpp::Float64>::type, float>::value);
    // std::cout << typeid(unwrapper<oatpp::Vector<oatpp::Vector<oatpp::Float64> > >::type).name();
    static_assert(std::is_same<unwrapper<oatpp::Vector<oatpp::Float64> >::type, std::vector<double> >::value);
    static_assert(std::is_same<
        unwrapper<oatpp::Vector<oatpp::Vector<oatpp::Float64> > >::type,
        std::vector<std::vector<double> >
    >::value);
    static_assert(std::is_same<
        unwrapper<oatpp::UnorderedMap<oatpp::Float64, oatpp::Float64> >::type,
        std::unordered_map<double, double>
    >::value);
    static_assert(std::is_same<
        unwrapper<oatpp::UnorderedMap<oatpp::Float64, oatpp::Vector<oatpp::List<oatpp::String> > > >::type,
        std::unordered_map<double, std::vector<std::list<std::string> > >
    >::value);
    static_assert(std::is_same<unwrapper<oatpp::String>::type, std::string>::value);
    static_assert(std::is_same<unwrapper<double>::type, double>::value);
    static_assert(std::is_same<unwrapper<oatpp::Any>::type, oatpp::Any>::value);
    static_assert(std::is_same<unwrapper<oatpp::Void>::type, oatpp::Void>::value);
    static_assert(std::is_same<unwrapper<oatpp::AbstractList>::type, std::list<oatpp::Void>>::value);

    oatpp::Vector<oatpp::Vector<oatpp::Float64> > v{{1, 2, 3, 4, 5}};
    auto v2 = deep_unwrapper(v);
    std::copy(v2.front().begin(), v2.front().end(), std::ostream_iterator<double>(std::cout, " "));
    // unwrapper<double>::type a = 1;
}

// BENCHMARK(BM_create_vector_and_resize_double);
// BENCHMARK(BM_create_vector_and_resize_std_string);
// BENCHMARK(BM_flat_transform);
// BENCHMARK(BM_std_transform_double);
// BENCHMARK(BM_std_transform_string);
// BENCHMARK(BM_flat_transform_with_pre_resize_double);
// BENCHMARK(BM_flat_transform_with_pre_resize_string);
// BENCHMARK(BM_flat_transform_with_back_insert_iterator);
// BENCHMARK(BM_flat_transform_with_back_insert_iterator_string);
// BENCHMARK(BM_std_transform_with_back_insert_iterator);
// BENCHMARK(BM_std_transform_with_back_insert_iterator_string);
BENCHMARK(oatppTest);

BENCHMARK_MAIN();
