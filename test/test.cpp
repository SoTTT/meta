//
// Created by H1773 on 25-6-20.
//

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <oatpp_helper.hpp>


TEST_CASE("test is_oatpp_wrapper", "[is_oatpp_wrapper]") {
    using namespace meta_operation::type_traits::oatpp;

    REQUIRE(is_oatpp_wrapper<oatpp::Vector<oatpp::Float64> >::value);
    REQUIRE(!is_oatpp_wrapper<std::vector<double> >::value);
    REQUIRE(is_oatpp_wrapper<oatpp::UnorderedMap<oatpp::String, oatpp::Float64> >::value);
    REQUIRE(is_oatpp_wrapper<oatpp::String>::value);
    REQUIRE(is_oatpp_wrapper<oatpp::Void>::value);
    REQUIRE(is_oatpp_wrapper<oatpp::Any>::value);

    // using namespace oatpp::data::mapping::type;
    // auto value = static_cast<__class::Collection::PolymorphicDispatcher const *>
    //         (oatpp::Vector<Float64>::Class::getType()->polymorphicDispatcher)
    //         ->createObject().cast<oatpp::Vector<Float64> >();


    // oatpp::Vector<oatpp::Vector<oatpp::Float64> > v{{1, 2, 3, 4, 5}};
    // auto v2 = deep_unwrapper(v);
    // std::copy(v2.front().begin(), v2.front().end(), std::ostream_iterator<double>(std::cout, " "));
    // unwrapper<double>::type a = 1;
}

TEST_CASE("test is_oatpp_container_wrapper", "[is_oatpp_container_wrapper]") {
    using namespace meta_operation::type_traits::oatpp;

    // meta_operation::type_traits::oatpp::is_oatpp_container_wrapper
    REQUIRE(is_oatpp_container_wrapper<oatpp::Vector<double> >::value);
    REQUIRE(!is_oatpp_container_wrapper<oatpp::Float64>::value);
    REQUIRE(!is_oatpp_container_wrapper<std::vector<double> >::value);
    REQUIRE(!is_oatpp_container_wrapper<double>::value);
    REQUIRE(is_oatpp_container_wrapper<oatpp::UnorderedMap<oatpp::String, oatpp::Float64> >::value);
    REQUIRE(!is_oatpp_container_wrapper<oatpp::String>::value);
    REQUIRE(!is_oatpp_container_wrapper<oatpp::Void>::value);
    REQUIRE(!is_oatpp_container_wrapper<oatpp::Any>::value);

    // meta_operation::type_traits::oatpp::is_oatpp_map_container_wrapper
    REQUIRE(is_oatpp_map_container_wrapper<oatpp::UnorderedMap<oatpp::String, oatpp::Float64> >::value);
    REQUIRE(!is_oatpp_map_container_wrapper<oatpp::Vector<oatpp::Float64> >::value);
}

TEST_CASE("test is_oatpp_primitive_wrapper", "[is_oatpp_primitive_wrapper]") {
    using namespace meta_operation::type_traits::oatpp;

    // meta_operation::type_traits::oatpp::is_oatpp_primitive_wrapper
    REQUIRE(is_oatpp_primitive_wrapper<oatpp::Float64>::value);
    REQUIRE(!is_oatpp_primitive_wrapper<oatpp::Vector<double> >::value);
    REQUIRE(!is_oatpp_primitive_wrapper<std::vector<double> >::value);
    REQUIRE(!is_oatpp_primitive_wrapper<double>::value);
    REQUIRE(is_oatpp_primitive_wrapper<oatpp::Int64>::value);
    REQUIRE(!is_oatpp_primitive_wrapper<oatpp::String>::value);
    REQUIRE(!is_oatpp_primitive_wrapper<oatpp::Void>::value);
    REQUIRE(!is_oatpp_primitive_wrapper<oatpp::Any>::value);
}

TEST_CASE("test unwrapper oatpp types", "[unwrapper]") {
    using namespace meta_operation::type_traits::oatpp;

    REQUIRE(std::is_same<unwrapper<double>::type, double>::value);
    REQUIRE(std::is_same<unwrapper<oatpp::Float64>::type, double>::value);
    REQUIRE(!std::is_same<unwrapper<oatpp::Float64>::type, float>::value);
    REQUIRE(std::is_same<unwrapper<oatpp::Vector<oatpp::Float64> >::type, std::vector<double> >::value);
    REQUIRE(std::is_same<
            unwrapper<oatpp::Vector<oatpp::Vector<oatpp::Float64> > >::type,
            std::vector<std::vector<double> >
            >::value);
    REQUIRE(std::is_same<
            unwrapper<oatpp::UnorderedMap<oatpp::Float64, oatpp::Float64> >::type,
            std::unordered_map<double, double>
            >::value);
    REQUIRE(std::is_same<
            unwrapper<oatpp::UnorderedMap<oatpp::Float64, oatpp::Vector<oatpp::List<oatpp::String> > > >::type,
            std::unordered_map<double, std::vector<std::list<std::string> > >
            >::value);
    REQUIRE(std::is_same<unwrapper<oatpp::String>::type, std::string>::value);
    REQUIRE(std::is_same<unwrapper<double>::type, double>::value);
    REQUIRE(std::is_same<unwrapper<oatpp::Any>::type, oatpp::Any>::value);
    REQUIRE(std::is_same<unwrapper<oatpp::Void>::type, oatpp::Void>::value);
    REQUIRE(std::is_same<unwrapper<oatpp::AbstractList>::type, std::list<oatpp::Void>>::value);
}

TEST_CASE("test deep_unwrapper oatpp::Vector<oatpp::Float64>", "[depp_unwrapper]") {
    // 生成随机数量的浮点数(50-150个)
    std::random_device rd;
    std::default_random_engine e(rd());
    std::uniform_int_distribution<int> count_dist(50, 150);
    const int count = count_dist(e);

    const oatpp::Vector<oatpp::Float64> floats = oatpp::Vector<oatpp::Float64>::createShared();

    std::uniform_real_distribution<> u(0, 100);

    floats->reserve(count);
    for (int i = 0; i < count; ++i) {
        floats->emplace_back(u(e));
    }

    std::vector<double> unwrapperContainer1;
    for (auto &it: *floats) {
        unwrapperContainer1.emplace_back(it);
    }

    auto unwrapperContainer2 = meta_operation::type_traits::oatpp::deep_unwrapper(floats);
    REQUIRE(std::is_same<decltype(unwrapperContainer2), std::vector<double>>::value);
    REQUIRE(unwrapperContainer1 == unwrapperContainer2);
}


TEST_CASE("test deep_unwrapper oatpp::Vector<oatpp::String>", "[deep_unwrapper]") {
    oatpp::Vector<oatpp::String> strings = oatpp::Vector<oatpp::String>::createShared();
    std::random_device rd;
    std::default_random_engine e(rd());
    std::uniform_int_distribution<int> char_dist('a', 'z');

    // 生成随机数量的字符串(50-150个)
    std::uniform_int_distribution<int> count_dist(50, 150);
    const int count = count_dist(e);

    // 每个字符串随机长度(5-20个字符)
    std::uniform_int_distribution<int> len_dist(5, 20);

    strings->reserve(count);
    for (int i = 0; i < count; ++i) {
        int len = len_dist(e);
        std::string s;
        s.reserve(len);
        for (int j = 0; j < len; ++j) {
            s.push_back(char_dist(e));
        }
        strings->emplace_back(s);
    }

    std::vector<std::string> unwrapperContainer1;
    for (auto &it: *strings) {
        unwrapperContainer1.emplace_back(it);
    }

    auto unwrapperContainer2 = meta_operation::type_traits::oatpp::deep_unwrapper(strings);
    REQUIRE(std::is_same<decltype(unwrapperContainer2), std::vector<std::string>>::value);
    REQUIRE(unwrapperContainer1 == unwrapperContainer2);
}