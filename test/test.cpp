#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <cstdint>
#include <meta.hpp>
#include <oatpp/core/macro/codegen.hpp>

// ---------------------------------------------------------------------------
// DTO 定义与 traits 全特化（DTO 定制点测试所需）
// ---------------------------------------------------------------------------

#include OATPP_CODEGEN_BEGIN(DTO)

class TestDto : public oatpp::DTO {
    DTO_INIT(TestDto, DTO)

    DTO_FIELD(oatpp::Int32, id);
    DTO_FIELD(oatpp::String, name);
};

// 刻意不提供任何 traits 特化：用于验证浅模式容器不触碰元素 traits
class RawDto : public oatpp::DTO {
    DTO_INIT(RawDto, DTO)

    DTO_FIELD(oatpp::Int32, value);
};

#include OATPP_CODEGEN_END(DTO)

struct TestStruct {
    int32_t id;
    std::string name;
};

namespace oatpp {
namespace meta {

template<>
struct traits<oatpp::Object<TestDto>> : dto_traits_base<TestDto, TestStruct> {
    template<typename Policy = null_to_throw>
    static UnwrapperType do_unwrapper(oatpp::Object<TestDto> const &value, Policy const & = {}) {
        TestStruct result{};
        if (value) {
            result.id = traits<oatpp::Int32>::do_unwrapper(value->id);
            result.name = traits<oatpp::String>::do_unwrapper(value->name);
        }
        return result;
    }

    static WrapperType do_wrapper(UnwrapperType const &value) {
        auto result = TestDto::createShared();
        result->id = traits<oatpp::Int32>::do_wrapper(value.id);
        result->name = traits<oatpp::String>::do_wrapper(value.name);
        return result;
    }
};

}
}

// ---------------------------------------------------------------------------
// 1. 类型分类与 is_xxx 标志
// ---------------------------------------------------------------------------

TEST_CASE("type_category classification", "[traits]") {
    using namespace oatpp::meta;

    // passthrough
    STATIC_REQUIRE(traits<int>::category == type_category::passthrough);
    STATIC_REQUIRE_FALSE(traits<int>::is_oatpp_type);
    STATIC_REQUIRE(traits<std::string>::category == type_category::passthrough);

    // primitive
    STATIC_REQUIRE(traits<oatpp::Int32>::category == type_category::primitive);
    STATIC_REQUIRE(traits<oatpp::Int32>::is_primitive);
    STATIC_REQUIRE(traits<oatpp::Int32>::is_scalar);
    STATIC_REQUIRE(traits<oatpp::Float64>::category == type_category::primitive);
    STATIC_REQUIRE(traits<oatpp::UInt64>::category == type_category::primitive);

    // scalar
    STATIC_REQUIRE(traits<oatpp::String>::category == type_category::scalar);
    STATIC_REQUIRE(traits<oatpp::Boolean>::category == type_category::scalar);
    STATIC_REQUIRE(traits<oatpp::String>::is_scalar);
    STATIC_REQUIRE_FALSE(traits<oatpp::String>::is_primitive);

    // container
    STATIC_REQUIRE(traits<oatpp::Vector<oatpp::Int32>>::category == type_category::container);
    STATIC_REQUIRE(traits<oatpp::Vector<oatpp::Int32>>::is_container);
    STATIC_REQUIRE(traits<oatpp::List<oatpp::String>>::category == type_category::container);
    STATIC_REQUIRE(traits<oatpp::UnorderedSet<oatpp::Int32>>::category == type_category::container);
    STATIC_REQUIRE(traits<oatpp::PairList<oatpp::String, oatpp::Int32>>::category == type_category::container);
    STATIC_REQUIRE(traits<oatpp::UnorderedMap<oatpp::String, oatpp::Int32>>::category == type_category::container);

    // opaque
    STATIC_REQUIRE(traits<oatpp::Void>::category == type_category::opaque);
    STATIC_REQUIRE(traits<oatpp::Any>::category == type_category::opaque);
    STATIC_REQUIRE(traits<oatpp::Void>::is_opaque);

    // object (DTO)
    STATIC_REQUIRE(traits<oatpp::Object<TestDto>>::category == type_category::object);
    STATIC_REQUIRE(traits<oatpp::Object<TestDto>>::is_object);
}

// ---------------------------------------------------------------------------
// 2. WrapperType / UnwrapperType
// ---------------------------------------------------------------------------

TEST_CASE("WrapperType and UnwrapperType", "[traits]") {
    using namespace oatpp::meta;

    // passthrough
    STATIC_REQUIRE((std::is_same<traits<int>::WrapperType, int>::value));
    STATIC_REQUIRE((std::is_same<traits<int>::UnwrapperType, int>::value));

    // primitive
    STATIC_REQUIRE((std::is_same<traits<oatpp::Int32>::WrapperType, oatpp::Int32>::value));
    STATIC_REQUIRE((std::is_same<traits<oatpp::Int32>::UnwrapperType, std::int32_t>::value));
    STATIC_REQUIRE((std::is_same<traits<oatpp::Float64>::WrapperType, oatpp::Float64>::value));
    STATIC_REQUIRE((std::is_same<traits<oatpp::Float64>::UnwrapperType, double>::value));

    // scalar
    STATIC_REQUIRE((std::is_same<traits<oatpp::String>::WrapperType, oatpp::String>::value));
    STATIC_REQUIRE((std::is_same<traits<oatpp::String>::UnwrapperType, std::string>::value));
    STATIC_REQUIRE((std::is_same<traits<oatpp::Boolean>::WrapperType, oatpp::Boolean>::value));
    STATIC_REQUIRE((std::is_same<traits<oatpp::Boolean>::UnwrapperType, bool>::value));

    // container
    STATIC_REQUIRE((std::is_same<traits<oatpp::Vector<oatpp::Int32>>::UnwrapperType,
                                 std::vector<std::int32_t>>::value));
    STATIC_REQUIRE((std::is_same<traits<oatpp::List<oatpp::String>>::UnwrapperType,
                                 std::list<std::string>>::value));
    STATIC_REQUIRE((std::is_same<traits<oatpp::UnorderedSet<oatpp::Int32>>::UnwrapperType,
                                 std::unordered_set<std::int32_t>>::value));
    STATIC_REQUIRE((std::is_same<traits<oatpp::PairList<oatpp::String, oatpp::Int32>>::UnwrapperType,
                                 std::list<std::pair<std::string, std::int32_t>>>::value));
    STATIC_REQUIRE((std::is_same<traits<oatpp::UnorderedMap<oatpp::String, oatpp::Int32>>::UnwrapperType,
                                 std::unordered_map<std::string, std::int32_t>>::value));

    // opaque
    STATIC_REQUIRE((std::is_same<traits<oatpp::Void>::WrapperType, oatpp::Void>::value));
    STATIC_REQUIRE((std::is_same<traits<oatpp::Void>::UnwrapperType, oatpp::Void>::value));

    // object
    STATIC_REQUIRE((std::is_same<traits<oatpp::Object<TestDto>>::WrapperType, oatpp::Object<TestDto>>::value));
    STATIC_REQUIRE((std::is_same<traits<oatpp::Object<TestDto>>::UnwrapperType, TestStruct>::value));
}

// ---------------------------------------------------------------------------
// 3. 标量解包（非 null）
// ---------------------------------------------------------------------------

TEST_CASE("scalar unwrapper — non-null values", "[traits]") {
    using namespace oatpp::meta;

    auto i32 = oatpp::Int32(42);
    REQUIRE(traits<oatpp::Int32>::do_unwrapper(i32) == 42);

    auto f64 = oatpp::Float64(3.14);
    REQUIRE(traits<oatpp::Float64>::do_unwrapper(f64) == Approx(3.14));

    auto str = oatpp::String("hello");
    REQUIRE(traits<oatpp::String>::do_unwrapper(str) == "hello");

    auto b = oatpp::Boolean(true);
    REQUIRE(traits<oatpp::Boolean>::do_unwrapper(b) == true);
}

// ---------------------------------------------------------------------------
// 4. null 标量行为
// ---------------------------------------------------------------------------

TEST_CASE("null scalar — default policy throws", "[traits][null]") {
    using namespace oatpp::meta;

    oatpp::Int32 null_int;
    REQUIRE_THROWS_AS(traits<oatpp::Int32>::do_unwrapper(null_int), null_unwrap_error);

    oatpp::String null_str;
    REQUIRE_THROWS_AS(traits<oatpp::String>::do_unwrapper(null_str), null_unwrap_error);

    oatpp::Boolean null_bool;
    REQUIRE_THROWS_AS(traits<oatpp::Boolean>::do_unwrapper(null_bool), null_unwrap_error);
}

TEST_CASE("null scalar — null_to_default policy", "[traits][null]") {
    using namespace oatpp::meta;

    oatpp::Int32 null_int;
    REQUIRE(traits<oatpp::Int32>::do_unwrapper(null_int, null_to_default{}) == 0);

    oatpp::String null_str;
    REQUIRE(traits<oatpp::String>::do_unwrapper(null_str, null_to_default{}).empty());

    oatpp::Boolean null_bool;
    REQUIRE(traits<oatpp::Boolean>::do_unwrapper(null_bool, null_to_default{}) == false);

    oatpp::Float64 null_f64;
    REQUIRE(traits<oatpp::Float64>::do_unwrapper(null_f64, null_to_default{}) == 0.0);
}

// ---------------------------------------------------------------------------
// 5. 容器解包（非 null）
// ---------------------------------------------------------------------------

TEST_CASE("container unwrapper — non-null", "[traits]") {
    using namespace oatpp::meta;

    // Vector
    auto vec = oatpp::Vector<oatpp::Int32>::createShared();
    vec->push_back(oatpp::Int32(1));
    vec->push_back(oatpp::Int32(2));
    vec->push_back(oatpp::Int32(3));
    auto v = traits<oatpp::Vector<oatpp::Int32>>::do_unwrapper(vec);
    REQUIRE(v == std::vector<int32_t>{1, 2, 3});

    // List
    auto lst = oatpp::List<oatpp::String>::createShared();
    lst->push_back(oatpp::String("a"));
    lst->push_back(oatpp::String("b"));
    auto l = traits<oatpp::List<oatpp::String>>::do_unwrapper(lst);
    REQUIRE(l == std::list<std::string>{"a", "b"});

    // UnorderedSet
    auto st = oatpp::UnorderedSet<oatpp::Int32>::createShared();
    st->insert(oatpp::Int32(10));
    st->insert(oatpp::Int32(20));
    auto s = traits<oatpp::UnorderedSet<oatpp::Int32>>::do_unwrapper(st);
    REQUIRE(s.size() == 2);
    REQUIRE(s.count(10) == 1);
    REQUIRE(s.count(20) == 1);

    // PairList
    auto pl = oatpp::PairList<oatpp::String, oatpp::Int32>::createShared();
    pl->emplace_back(oatpp::String("x"), oatpp::Int32(100));
    pl->emplace_back(oatpp::String("y"), oatpp::Int32(200));
    auto p = traits<oatpp::PairList<oatpp::String, oatpp::Int32>>::do_unwrapper(pl);
    REQUIRE(p.size() == 2);
    REQUIRE(p.front().first == "x");
    REQUIRE(p.front().second == 100);

    // UnorderedMap
    auto mp = oatpp::UnorderedMap<oatpp::String, oatpp::Int32>::createShared();
    mp->emplace(oatpp::String("k1"), oatpp::Int32(1));
    mp->emplace(oatpp::String("k2"), oatpp::Int32(2));
    auto m = traits<oatpp::UnorderedMap<oatpp::String, oatpp::Int32>>::do_unwrapper(mp);
    REQUIRE(m.size() == 2);
    REQUIRE(m.at("k1") == 1);
    REQUIRE(m.at("k2") == 2);
}

// ---------------------------------------------------------------------------
// 6. 容器 null -> 空容器（内建约定）
// ---------------------------------------------------------------------------

TEST_CASE("null container -> empty container", "[traits][null]") {
    using namespace oatpp::meta;

    oatpp::Vector<oatpp::Int32> null_vec;
    REQUIRE(traits<oatpp::Vector<oatpp::Int32>>::do_unwrapper(null_vec).empty());

    oatpp::List<oatpp::String> null_lst;
    REQUIRE(traits<oatpp::List<oatpp::String>>::do_unwrapper(null_lst).empty());

    oatpp::UnorderedSet<oatpp::Int32> null_st;
    REQUIRE(traits<oatpp::UnorderedSet<oatpp::Int32>>::do_unwrapper(null_st).empty());

    oatpp::PairList<oatpp::String, oatpp::Int32> null_pl;
    REQUIRE(traits<oatpp::PairList<oatpp::String, oatpp::Int32>>::do_unwrapper(null_pl).empty());

    oatpp::UnorderedMap<oatpp::String, oatpp::Int32> null_mp;
    REQUIRE(traits<oatpp::UnorderedMap<oatpp::String, oatpp::Int32>>::do_unwrapper(null_mp).empty());
}

// ---------------------------------------------------------------------------
// 7. 嵌套容器
// ---------------------------------------------------------------------------

TEST_CASE("nested containers", "[traits]") {
    using namespace oatpp::meta;

    // Vector<Vector<Int32>>
    auto vv = oatpp::Vector<oatpp::Vector<oatpp::Int32>>::createShared();
    {
        auto inner = oatpp::Vector<oatpp::Int32>::createShared();
        inner->push_back(oatpp::Int32(1));
        inner->push_back(oatpp::Int32(2));
        vv->push_back(inner);
    }
    {
        auto inner = oatpp::Vector<oatpp::Int32>::createShared();
        inner->push_back(oatpp::Int32(3));
        vv->push_back(inner);
    }
    auto result_vv = traits<oatpp::Vector<oatpp::Vector<oatpp::Int32>>>::do_unwrapper(vv);
    REQUIRE(result_vv.size() == 2);
    REQUIRE(result_vv[0] == std::vector<int32_t>{1, 2});
    REQUIRE(result_vv[1] == std::vector<int32_t>{3});

    // UnorderedMap<String, Vector<Int32>>
    auto mv = oatpp::UnorderedMap<oatpp::String, oatpp::Vector<oatpp::Int32>>::createShared();
    {
        auto inner = oatpp::Vector<oatpp::Int32>::createShared();
        inner->push_back(oatpp::Int32(10));
        inner->push_back(oatpp::Int32(20));
        mv->emplace(oatpp::String("a"), inner);
    }
    auto result_mv = traits<oatpp::UnorderedMap<oatpp::String, oatpp::Vector<oatpp::Int32>>>::do_unwrapper(mv);
    REQUIRE(result_mv.size() == 1);
    REQUIRE(result_mv.at("a") == std::vector<int32_t>{10, 20});
}

// ---------------------------------------------------------------------------
// 8. 容器内 null 元素
// ---------------------------------------------------------------------------

TEST_CASE("container with null elements — default policy throws", "[traits][null]") {
    using namespace oatpp::meta;

    auto vec = oatpp::Vector<oatpp::Int32>::createShared();
    vec->push_back(oatpp::Int32(1));
    vec->push_back(oatpp::Int32()); // null
    vec->push_back(oatpp::Int32(3));

    REQUIRE_THROWS_AS(traits<oatpp::Vector<oatpp::Int32>>::do_unwrapper(vec), null_unwrap_error);
}

TEST_CASE("container with null elements — null_to_default policy", "[traits][null]") {
    using namespace oatpp::meta;

    auto vec = oatpp::Vector<oatpp::Int32>::createShared();
    vec->push_back(oatpp::Int32(1));
    vec->push_back(oatpp::Int32()); // null -> 0
    vec->push_back(oatpp::Int32(3));

    auto result = traits<oatpp::Vector<oatpp::Int32>>::do_unwrapper(vec, null_to_default{});
    REQUIRE(result == std::vector<int32_t>{1, 0, 3});
}

// ---------------------------------------------------------------------------
// 9. 反向包装（do_wrapper）
// ---------------------------------------------------------------------------

TEST_CASE("do_wrapper — scalars", "[traits]") {
    using namespace oatpp::meta;

    auto i32 = traits<oatpp::Int32>::do_wrapper(42);
    REQUIRE(traits<oatpp::Int32>::do_unwrapper(i32) == 42);

    auto str = traits<oatpp::String>::do_wrapper(std::string("world"));
    REQUIRE(traits<oatpp::String>::do_unwrapper(str) == "world");

    auto b = traits<oatpp::Boolean>::do_wrapper(false);
    REQUIRE(b != nullptr);
    REQUIRE(traits<oatpp::Boolean>::do_unwrapper(b) == false);

    auto f64 = traits<oatpp::Float64>::do_wrapper(2.718);
    REQUIRE(traits<oatpp::Float64>::do_unwrapper(f64) == Approx(2.718));
}

TEST_CASE("do_wrapper — containers", "[traits]") {
    using namespace oatpp::meta;

    // Vector
    auto vec = traits<oatpp::Vector<oatpp::Int32>>::do_wrapper(std::vector<int32_t>{7, 8, 9});
    auto unwrapped_vec = traits<oatpp::Vector<oatpp::Int32>>::do_unwrapper(vec);
    REQUIRE(unwrapped_vec == std::vector<int32_t>{7, 8, 9});

    // UnorderedMap
    std::unordered_map<std::string, int32_t> m{{"a", 1}, {"b", 2}};
    auto mp = traits<oatpp::UnorderedMap<oatpp::String, oatpp::Int32>>::do_wrapper(m);
    auto unwrapped_mp = traits<oatpp::UnorderedMap<oatpp::String, oatpp::Int32>>::do_unwrapper(mp);
    REQUIRE(unwrapped_mp.size() == 2);
    REQUIRE(unwrapped_mp.at("a") == 1);
    REQUIRE(unwrapped_mp.at("b") == 2);

    // PairList
    std::list<std::pair<std::string, int32_t>> pl{{"x", 10}, {"y", 20}};
    auto pairlist = traits<oatpp::PairList<oatpp::String, oatpp::Int32>>::do_wrapper(pl);
    auto unwrapped_pl = traits<oatpp::PairList<oatpp::String, oatpp::Int32>>::do_unwrapper(pairlist);
    REQUIRE(unwrapped_pl.size() == 2);
    auto it = unwrapped_pl.begin();
    REQUIRE(it->first == "x");
    REQUIRE(it->second == 10);
}

TEST_CASE("do_wrapper — nested containers", "[traits]") {
    using namespace oatpp::meta;

    std::vector<std::vector<int32_t>> vv{{1, 2}, {3}};
    auto wrapped = traits<oatpp::Vector<oatpp::Vector<oatpp::Int32>>>::do_wrapper(vv);
    auto unwrapped = traits<oatpp::Vector<oatpp::Vector<oatpp::Int32>>>::do_unwrapper(wrapped);
    REQUIRE(unwrapped.size() == 2);
    REQUIRE(unwrapped[0].size() == 2);
    REQUIRE(unwrapped[0][0] == 1);
    REQUIRE(unwrapped[0][1] == 2);
    REQUIRE(unwrapped[1].size() == 1);
    REQUIRE(unwrapped[1][0] == 3);
}

// ---------------------------------------------------------------------------
// 10. DTO 定制点
// ---------------------------------------------------------------------------

TEST_CASE("DTO custom traits — unwrapper", "[traits][dto]") {
    using namespace oatpp::meta;

    auto dto = TestDto::createShared();
    dto->id = 42;
    dto->name = "alice";

    auto s = traits<oatpp::Object<TestDto>>::do_unwrapper(dto);
    REQUIRE(s.id == 42);
    REQUIRE(s.name == "alice");
}

TEST_CASE("DTO custom traits — wrapper", "[traits][dto]") {
    using namespace oatpp::meta;

    TestStruct s{42, "alice"};
    auto dto = traits<oatpp::Object<TestDto>>::do_wrapper(s);
    REQUIRE(traits<oatpp::Int32>::do_unwrapper(dto->id) == 42);
    REQUIRE(traits<oatpp::String>::do_unwrapper(dto->name) == "alice");
}

TEST_CASE("DTO in container", "[traits][dto]") {
    using namespace oatpp::meta;

    auto vec = oatpp::Vector<oatpp::Object<TestDto>>::createShared();
    {
        auto dto = TestDto::createShared();
        dto->id = 1;
        dto->name = "a";
        vec->push_back(dto);
    }
    {
        auto dto = TestDto::createShared();
        dto->id = 2;
        dto->name = "b";
        vec->push_back(dto);
    }

    auto result = traits<oatpp::Vector<oatpp::Object<TestDto>>>::do_unwrapper(vec);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0].id == 1);
    REQUIRE(result[0].name == "a");
    REQUIRE(result[1].id == 2);
    REQUIRE(result[1].name == "b");
}

// ---------------------------------------------------------------------------
// 11. passthrough 类型（非 oatpp 类型原样穿透）
// ---------------------------------------------------------------------------

TEST_CASE("passthrough types", "[traits]") {
    using namespace oatpp::meta;

    int x = 123;
    REQUIRE(traits<int>::do_unwrapper(x) == 123);
    REQUIRE(traits<int>::do_wrapper(456) == 456);
}

// ---------------------------------------------------------------------------
// 12. opaque 类型（Void / Any）
// ---------------------------------------------------------------------------

TEST_CASE("opaque types — passthrough", "[traits]") {
    using namespace oatpp::meta;

    oatpp::Void v;
    REQUIRE_NOTHROW(traits<oatpp::Void>::do_unwrapper(v));

    oatpp::Any a;
    REQUIRE_NOTHROW(traits<oatpp::Any>::do_unwrapper(a));
}

// ---------------------------------------------------------------------------
// 13. 声明式类型分派策略组合器 (policy::combine)
// ---------------------------------------------------------------------------

TEST_CASE("policy::combine — per-type null policy", "[traits][null][policy]") {
    using namespace oatpp::meta;

    // Int32 null -> default, String null -> throw
    {
        oatpp::Int32 null_i32;
        oatpp::String null_str;

        using mixed = policy::combine<
            policy::for_type<std::int32_t, null_to_default>,
            policy::otherwise<null_to_throw>
        >;

        REQUIRE(traits<oatpp::Int32>::do_unwrapper(null_i32, mixed{}) == 0);
        REQUIRE_THROWS_AS(traits<oatpp::String>::do_unwrapper(null_str, mixed{}), null_unwrap_error);
    }

    // String null -> default, 其余抛异常
    {
        oatpp::Int32 null_i32;
        oatpp::String null_str;

        using mixed = policy::combine<
            policy::for_type<std::string, null_to_default>,
            policy::otherwise<null_to_throw>
        >;

        REQUIRE_THROWS_AS(traits<oatpp::Int32>::do_unwrapper(null_i32, mixed{}), null_unwrap_error);
        REQUIRE(traits<oatpp::String>::do_unwrapper(null_str, mixed{}).empty());
    }
}

TEST_CASE("policy::combine — in container recursion", "[traits][null][policy]") {
    using namespace oatpp::meta;

    // PairList<String, Int32>: String null -> default (empty), Int32 null -> throw
    using strict_int32_default_string = policy::combine<
        policy::for_type<std::int32_t, null_to_throw>,
        policy::for_type<std::string, null_to_default>,
        policy::otherwise<null_to_throw>
    >;

    // key 正常，value null -> throw
    {
        auto pl = oatpp::PairList<oatpp::String, oatpp::Int32>::createShared();
        pl->emplace_back(oatpp::String("ok"), oatpp::Int32()); // null Int32
        REQUIRE_THROWS_AS(
            (traits<oatpp::PairList<oatpp::String, oatpp::Int32>>::do_unwrapper(
                pl, strict_int32_default_string{})),
            null_unwrap_error);
    }

    // key null -> default (empty), value 正常
    {
        auto pl = oatpp::PairList<oatpp::String, oatpp::Int32>::createShared();
        pl->emplace_back(oatpp::String(), oatpp::Int32(42)); // null String
        auto result = traits<oatpp::PairList<oatpp::String, oatpp::Int32>>::do_unwrapper(
            pl, strict_int32_default_string{});
        REQUIRE(result.size() == 1);
        REQUIRE(result.front().first.empty());
        REQUIRE(result.front().second == 42);
    }

    // 混合：key null (default), value null (throw)
    {
        auto pl = oatpp::PairList<oatpp::String, oatpp::Int32>::createShared();
        pl->emplace_back(oatpp::String(), oatpp::Int32()); // 两者都 null
        REQUIRE_THROWS_AS(
            (traits<oatpp::PairList<oatpp::String, oatpp::Int32>>::do_unwrapper(
                pl, strict_int32_default_string{})),
            null_unwrap_error);
    }
}

TEST_CASE("policy::combine — otherwise catches all", "[traits][null][policy]") {
    using namespace oatpp::meta;

    using fallback_default = policy::combine<
        policy::otherwise<null_to_default>
    >;

    oatpp::Int32 null_i32;
    oatpp::String null_str;
    oatpp::Boolean null_bool;

    REQUIRE(traits<oatpp::Int32>::do_unwrapper(null_i32, fallback_default{}) == 0);
    REQUIRE(traits<oatpp::String>::do_unwrapper(null_str, fallback_default{}).empty());
    REQUIRE(traits<oatpp::Boolean>::do_unwrapper(null_bool, fallback_default{}) == false);
}

TEST_CASE("policy::combine — order matters (first match wins)", "[traits][null][policy]") {
    using namespace oatpp::meta;

    // 先放 otherwise 再放 for_type：for_type 永远不会被匹配到
    using wrong_order = policy::combine<
        policy::otherwise<null_to_default>,
        policy::for_type<std::int32_t, null_to_throw>
    >;

    oatpp::Int32 null_i32;
    // 因为 otherwise 在前面先匹配，Int32 也走 default
    REQUIRE(traits<oatpp::Int32>::do_unwrapper(null_i32, wrong_order{}) == 0);
}

// ---------------------------------------------------------------------------
// 14. 浅模式（recursion::shallow）：只转换最外层容器，元素保持 oatpp 包装
// ---------------------------------------------------------------------------

TEST_CASE("shallow — type-level element types", "[traits][shallow]") {
    using namespace oatpp::meta;

    // 外层容器转 std，元素保持 oatpp 包装
    STATIC_REQUIRE((std::is_same<shallow_traits<oatpp::Vector<oatpp::Int32>>::UnwrapperType,
                                 std::vector<oatpp::Int32>>::value));
    STATIC_REQUIRE((std::is_same<shallow_traits<oatpp::List<oatpp::String>>::UnwrapperType,
                                 std::list<oatpp::String>>::value));
    STATIC_REQUIRE((std::is_same<shallow_traits<oatpp::UnorderedSet<oatpp::Int32>>::UnwrapperType,
                                 std::unordered_set<oatpp::Int32>>::value));
    STATIC_REQUIRE((std::is_same<shallow_traits<oatpp::PairList<oatpp::String, oatpp::Int32>>::UnwrapperType,
                                 std::list<std::pair<oatpp::String, oatpp::Int32>>>::value));
    STATIC_REQUIRE((std::is_same<shallow_traits<oatpp::UnorderedMap<oatpp::String, oatpp::Int32>>::UnwrapperType,
                                 std::unordered_map<oatpp::String, oatpp::Int32>>::value));

    // 嵌套：只解最外层，内层仍是 oatpp 容器
    STATIC_REQUIRE((std::is_same<
        shallow_traits<oatpp::Vector<oatpp::Vector<oatpp::Int32>>>::UnwrapperType,
        std::vector<oatpp::Vector<oatpp::Int32>>>::value));

    // 叶子无递归：shallow 等价 deep
    STATIC_REQUIRE((std::is_same<traits<oatpp::String, recursion::shallow>::UnwrapperType,
                                 std::string>::value));
    STATIC_REQUIRE((std::is_same<traits<oatpp::Int32, recursion::shallow>::UnwrapperType,
                                 std::int32_t>::value));

    // 默认参数与显式 deep 等价
    STATIC_REQUIRE((std::is_same<
        traits<oatpp::Vector<oatpp::Int32>>::UnwrapperType,
        traits<oatpp::Vector<oatpp::Int32>, recursion::deep>::UnwrapperType>::value));

    // 分类标志与 deep 一致（Recursion 只影响元素类型，不影响类型分类）
    STATIC_REQUIRE(shallow_traits<oatpp::Vector<oatpp::Int32>>::is_container);
    STATIC_REQUIRE(shallow_traits<oatpp::Vector<oatpp::Int32>>::category == type_category::container);
}

TEST_CASE("shallow — unwrap keeps element wrappers", "[traits][shallow]") {
    using namespace oatpp::meta;

    auto vec = oatpp::Vector<oatpp::Int32>::createShared();
    vec->push_back(oatpp::Int32(1));
    vec->push_back(oatpp::Int32()); // null
    vec->push_back(oatpp::Int32(3));

    std::vector<oatpp::Int32> result = shallow_traits<oatpp::Vector<oatpp::Int32>>::do_unwrapper(vec);
    REQUIRE(result.size() == 3);
    REQUIRE(result[0].getValue(0) == 1);
    // 浅模式不解包元素，null 元素原样保留、不进入 Policy
    REQUIRE(result[1].get() == nullptr);
    REQUIRE(result[2].getValue(0) == 3);

    // 对比：deep 默认策略对同样的 null 元素抛异常
    REQUIRE_THROWS_AS(traits<oatpp::Vector<oatpp::Int32>>::do_unwrapper(vec), null_unwrap_error);
}

TEST_CASE("shallow — nested: null inner container kept null", "[traits][shallow]") {
    using namespace oatpp::meta;

    auto vv = oatpp::Vector<oatpp::Vector<oatpp::Int32>>::createShared();
    {
        auto inner = oatpp::Vector<oatpp::Int32>::createShared();
        inner->push_back(oatpp::Int32(1));
        inner->push_back(oatpp::Int32(2));
        vv->push_back(inner);
    }
    vv->push_back(oatpp::Vector<oatpp::Int32>()); // null 内层

    std::vector<oatpp::Vector<oatpp::Int32>> result =
            shallow_traits<oatpp::Vector<oatpp::Vector<oatpp::Int32>>>::do_unwrapper(vv);
    REQUIRE(result.size() == 2);
    {
        auto const &inner = *result[0].get();
        REQUIRE(inner.size() == 2);
        REQUIRE(inner[0].getValue(0) == 1);
        REQUIRE(inner[1].getValue(0) == 2);
    }
    // 浅模式不看进元素：null 子容器保持 null，不会被转成空容器
    REQUIRE(result[1].get() == nullptr);

    // 对比：deep 下 null 内层容器会被转成空容器
    auto deep_result = traits<oatpp::Vector<oatpp::Vector<oatpp::Int32>>>::do_unwrapper(vv);
    REQUIRE(deep_result.size() == 2);
    REQUIRE(deep_result[1].empty());
}

TEST_CASE("shallow — container of DTO needs no traits specialization", "[traits][shallow][dto]") {
    using namespace oatpp::meta;

    // RawDto 没有任何 traits 特化；浅模式不触碰 traits<Object<RawDto>>，因此可编译
    STATIC_REQUIRE((std::is_same<
        shallow_traits<oatpp::Vector<oatpp::Object<RawDto>>>::UnwrapperType,
        std::vector<oatpp::Object<RawDto>>>::value));

    auto vec = oatpp::Vector<oatpp::Object<RawDto>>::createShared();
    {
        auto dto = RawDto::createShared();
        dto->value = 7;
        vec->push_back(dto);
    }
    vec->push_back(oatpp::Object<RawDto>()); // null DTO

    std::vector<oatpp::Object<RawDto>> result =
            shallow_traits<oatpp::Vector<oatpp::Object<RawDto>>>::do_unwrapper(vec);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0]->value.getValue(0) == 7);
    REQUIRE(result[1].get() == nullptr);
}

TEST_CASE("shallow — wrapper round trip", "[traits][shallow]") {
    using namespace oatpp::meta;

    // Vector<Int32>
    std::vector<oatpp::Int32> v{oatpp::Int32(1), oatpp::Int32(2), oatpp::Int32()};
    auto wrapped = shallow_traits<oatpp::Vector<oatpp::Int32>>::do_wrapper(v);
    REQUIRE(wrapped.get() != nullptr); // 恒产生非 null 外层容器
    {
        auto const &inner = *wrapped.get();
        REQUIRE(inner.size() == 3);
        REQUIRE(inner[0].getValue(0) == 1);
        REQUIRE(inner[2].get() == nullptr); // 元素 null 原样保留
    }
    auto back = shallow_traits<oatpp::Vector<oatpp::Int32>>::do_unwrapper(wrapped);
    REQUIRE(back.size() == 3);
    REQUIRE(back[1].getValue(0) == 2);

    // List<String>
    std::list<oatpp::String> sl{oatpp::String("a"), oatpp::String("b")};
    auto wrapped_l = shallow_traits<oatpp::List<oatpp::String>>::do_wrapper(sl);
    REQUIRE(wrapped_l.get() != nullptr);
    REQUIRE(shallow_traits<oatpp::List<oatpp::String>>::do_unwrapper(wrapped_l).size() == 2);

    // UnorderedSet<Int32>
    std::unordered_set<oatpp::Int32> ss{oatpp::Int32(5)};
    auto wrapped_s = shallow_traits<oatpp::UnorderedSet<oatpp::Int32>>::do_wrapper(ss);
    REQUIRE(wrapped_s.get() != nullptr);
    REQUIRE(shallow_traits<oatpp::UnorderedSet<oatpp::Int32>>::do_unwrapper(wrapped_s).size() == 1);

    // PairList<String, Int32>
    std::list<std::pair<oatpp::String, oatpp::Int32>> pl{
        {oatpp::String("x"), oatpp::Int32(10)}
    };
    auto wrapped_pl = shallow_traits<oatpp::PairList<oatpp::String, oatpp::Int32>>::do_wrapper(pl);
    REQUIRE(wrapped_pl.get() != nullptr);
    {
        auto back_pl = shallow_traits<oatpp::PairList<oatpp::String, oatpp::Int32>>::do_unwrapper(wrapped_pl);
        REQUIRE(back_pl.size() == 1);
        REQUIRE(back_pl.front().second.getValue(0) == 10);
    }

    // UnorderedMap<String, Int32>（key 为 oatpp::String，依赖 oatpp 提供的 std::hash）
    std::unordered_map<oatpp::String, oatpp::Int32> m{
        {oatpp::String("a"), oatpp::Int32(1)},
        {oatpp::String("b"), oatpp::Int32(2)}
    };
    auto wrapped_map = shallow_traits<oatpp::UnorderedMap<oatpp::String, oatpp::Int32>>::do_wrapper(m);
    REQUIRE(wrapped_map.get() != nullptr);
    {
        auto back_map = shallow_traits<oatpp::UnorderedMap<oatpp::String, oatpp::Int32>>::do_unwrapper(wrapped_map);
        REQUIRE(back_map.size() == 2);
        REQUIRE(back_map.at(oatpp::String("a")).getValue(0) == 1);
        REQUIRE(back_map.at(oatpp::String("b")).getValue(0) == 2);
    }
}

TEST_CASE("shallow — null container -> empty", "[traits][shallow][null]") {
    using namespace oatpp::meta;

    oatpp::Vector<oatpp::Int32> null_vec;
    REQUIRE(shallow_traits<oatpp::Vector<oatpp::Int32>>::do_unwrapper(null_vec).empty());

    oatpp::UnorderedMap<oatpp::String, oatpp::Int32> null_map;
    REQUIRE(shallow_traits<oatpp::UnorderedMap<oatpp::String, oatpp::Int32>>::do_unwrapper(null_map).empty());

    oatpp::List<oatpp::String> null_list;
    REQUIRE(shallow_traits<oatpp::List<oatpp::String>>::do_unwrapper(null_list).empty());
}
