//
// Created by H1773 on 25-6-7.
//

#ifndef OATPP_HELPER_HPP
#define OATPP_HELPER_HPP

#include <list>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <oatpp/core/Types.hpp>

namespace oatpp {
    namespace meta_operation {
        // ------------------------------------------------------------------
        // null 语义三层模型
        //
        // 1. 容器 null（Vector/List/... 本身为空）-> 空容器
        //    库内建约定，不进入策略接口（null 数组与空数组的区分在业务上几乎总是
        //    无意义；真正在乎这一区分的使用者应走 optional 映射，那是类型层的事）。
        // 2. 标量 null（任意深度，含容器内的元素）-> 由 Policy 决定
        //    默认 null_to_throw（fail-fast，逼迫调用方显式思考 null 语义）；
        //    宽容语义必须显式写出：do_unwrapper(value, null_to_default{})。
        //    自定义策略只需提供：template<typename U> U on_null_scalar() const。
        // 3. 嵌套对象（DTO）-> 库结构性不插手
        //    DTOWrapper 默认 static_assert（见文件末尾），逐字段的可空性与
        //    null 处理由用户的 traits 特化全权负责；策略传播不穿越用户代码。
        // ------------------------------------------------------------------

        /**
         * @brief 标量解包遇到 null 包装时抛出的异常
         */
        class null_unwrap_error : public std::runtime_error {
        public:
            explicit null_unwrap_error(char const *message) : std::runtime_error(message) {
            }
        };

        /**
         * @brief 默认 null 策略：标量 null 视为数据契约违反，抛出 null_unwrap_error
         */
        struct null_to_throw {
            template<typename U>
            [[noreturn]] U on_null_scalar() const {
                throw null_unwrap_error("unwrapping null oatpp scalar wrapper");
            }
        };

        /**
         * @brief 宽容 null 策略：标量 null 映射为默认值构造的结果（0/空串/false），
         *        会丢失 null 与零值的区分，需调用方显式传入
         */
        struct null_to_default {
            template<typename U>
            constexpr U on_null_scalar() const {
                return U{};
            }
        };

        /**
         * @brief 包装类型的互斥分类
         */
        enum class type_category {
            passthrough, // 非 oatpp 类型，原样穿透
            primitive, // Primitive<T, Clazz> 数值原语（Int8 ... Float64）
            scalar, // 非算术单值叶子：String / Boolean / Enum
            container, // 容器：Vector / List / UnorderedSet / PairList / UnorderedMap
            object, // DTO 对象（用户定制点）
            opaque // Void / Any：无法静态解包
        };

        /**
         * @brief 所有 traits 特化的公共基类：只需指定互斥的 type_category，
         *        全部 is_xxx 特征常量由此派生（非法组合不可表达）
         * @tparam C 该类型的分类
         */
        template<type_category C>
        struct traits_base {
            static constexpr type_category category = C;

            static constexpr bool is_oatpp_type = C != type_category::passthrough;

            static constexpr bool is_primitive = C == type_category::primitive;

            static constexpr bool is_scalar =
                    C == type_category::primitive || C == type_category::scalar;

            static constexpr bool is_container = C == type_category::container;

            static constexpr bool is_object = C == type_category::object;

            static constexpr bool is_opaque = C == type_category::opaque;
        };

        /**
         * @brief oatpp 包装类型特征的基础模板（默认分支：非包装类型原样穿透）
         * @tparam T 待鉴别的类型
         * @memberof WrapperType 包装类型自身
         * @memberof UnwrapperType 解包装后的类型
         * @memberof do_unwrapper 运行时解包装；null 语义见上方三层模型，Policy 沿递归传递
         * @memberof do_wrapper 运行时包装（UnwrapperType -> WrapperType）；恒产生非 null 包装
         */
        template<typename T>
        struct traits : traits_base<type_category::passthrough> {
            using WrapperType = T;

            using UnwrapperType = T;

            template<typename Policy = null_to_throw>
            static constexpr UnwrapperType do_unwrapper(T const &value, Policy const & = {}) {
                return value;
            }

            static constexpr WrapperType do_wrapper(UnwrapperType const &value) {
                return value;
            }
        };

        /**
         * @brief oatpp 数值原语：Int8/UInt8 ... Int64/UInt64/Float32/Float64
         *        （共 10 个 typedef，均为 Primitive<T, Clazz> 的实例）-> 对应算术类型
         */
        template<typename T, typename Clazz>
        struct traits<data::mapping::type::Primitive<T, Clazz> > : traits_base<type_category::primitive> {
            using WrapperType = data::mapping::type::Primitive<T, Clazz>;

            using UnwrapperType = T;

            template<typename Policy = null_to_throw>
            static UnwrapperType do_unwrapper(data::mapping::type::Primitive<T, Clazz> const &value,
                                              Policy const &policy = {}) {
                if (value.get() == nullptr) {
                    return policy.template on_null_scalar<UnwrapperType>();
                }
                return value.getValue(UnwrapperType{});
            }

            static WrapperType do_wrapper(UnwrapperType const &value) {
                return WrapperType(value);
            }
        };

        /**
         * @brief oatpp::Void：无值语义，原样穿透
         */
        template<>
        struct traits<Void> : traits_base<type_category::opaque> {
            using WrapperType = Void;

            using UnwrapperType = Void;

            template<typename Policy = null_to_throw>
            static UnwrapperType do_unwrapper(Void const &value, Policy const & = {}) {
                return value;
            }

            static WrapperType do_wrapper(Void const &value) {
                return value;
            }
        };

        /**
         * @brief oatpp::Any：多态持有任意包装类型，无法静态解包，原样穿透
         */
        template<>
        struct traits<Any> : traits_base<type_category::opaque> {
            using WrapperType = Any;

            using UnwrapperType = Any;

            template<typename Policy = null_to_throw>
            static UnwrapperType do_unwrapper(Any const &value, Policy const & = {}) {
                return value;
            }

            static WrapperType do_wrapper(Any const &value) {
                return value;
            }
        };

        /**
         * @brief oatpp::String -> std::string
         */
        template<>
        struct traits<String> : traits_base<type_category::scalar> {
            using WrapperType = String;

            using UnwrapperType = std::string;

            template<typename Policy = null_to_throw>
            static UnwrapperType do_unwrapper(String const &value, Policy const &policy = {}) {
                if (value.get() == nullptr) {
                    return policy.template on_null_scalar<UnwrapperType>();
                }
                return *value.get();
            }

            static WrapperType do_wrapper(UnwrapperType const &value) {
                return WrapperType(value);
            }
        };

        /**
         * @brief oatpp::Boolean -> bool
         */
        template<>
        struct traits<Boolean> : traits_base<type_category::scalar> {
            using WrapperType = Boolean;

            using UnwrapperType = bool;

            template<typename Policy = null_to_throw>
            static UnwrapperType do_unwrapper(Boolean const &value, Policy const &policy = {}) {
                if (value.get() == nullptr) {
                    return policy.template on_null_scalar<UnwrapperType>();
                }
                return *value.get();
            }

            static WrapperType do_wrapper(UnwrapperType const &value) {
                return WrapperType(value);
            }
        };

        /**
         * @brief oatpp 枚举包装：Enum<T> 及其 AsString/AsNumber/NotNull 变体
         *        （均为 EnumObjectWrapper<T, Interpreter> 的实例）-> C++ 枚举类型 T 本身；
         *        解释器只影响序列化表现，不改变解包后的值类型
         */
        template<typename T, typename Interpreter>
        struct traits<data::mapping::type::EnumObjectWrapper<T, Interpreter> >
                : traits_base<type_category::scalar> {
            using WrapperType = data::mapping::type::EnumObjectWrapper<T, Interpreter>;

            using UnwrapperType = T;

            template<typename Policy = null_to_throw>
            static UnwrapperType do_unwrapper(
                data::mapping::type::EnumObjectWrapper<T, Interpreter> const &value,
                Policy const &policy = {}) {
                if (value.get() == nullptr) {
                    return policy.template on_null_scalar<UnwrapperType>();
                }
                return *value.get();
            }

            static WrapperType do_wrapper(UnwrapperType const &value) {
                return WrapperType(value);
            }
        };

        /**
         * @brief oatpp::Vector<T> -> std::vector<递归解包后的元素类型>
         *        null 容器 -> 空 vector（内建约定）；null 元素 -> 由 Policy 决定
         */
        template<typename T>
        struct traits<Vector<T> > : traits_base<type_category::container> {
            using WrapperType = Vector<T>;

            using UnwrapperType = std::vector<typename traits<T>::UnwrapperType>;

            template<typename Policy = null_to_throw>
            static UnwrapperType do_unwrapper(Vector<T> const &value, Policy const &policy = {}) {
                UnwrapperType result;
                if (value.get() == nullptr) {
                    return result;
                }
                result.reserve(value->size());
                for (auto const &element: *value.get()) {
                    result.push_back(traits<T>::do_unwrapper(element, policy));
                }
                return result;
            }

            static WrapperType do_wrapper(UnwrapperType const &value) {
                WrapperType result = WrapperType::createShared();
                result->reserve(value.size());
                for (auto const &element: value) {
                    result->push_back(traits<T>::do_wrapper(element));
                }
                return result;
            }
        };

        /**
         * @brief oatpp::List<T> -> std::list<递归解包后的元素类型>
         *        注意：std::list 无 reserve/operator[]，只能 push_back（历史缺陷 CF02）；
         *        null 容器 -> 空 list（内建约定）
         */
        template<typename T>
        struct traits<List<T> > : traits_base<type_category::container> {
            using WrapperType = List<T>;

            using UnwrapperType = std::list<typename traits<T>::UnwrapperType>;

            template<typename Policy = null_to_throw>
            static UnwrapperType do_unwrapper(List<T> const &value, Policy const &policy = {}) {
                UnwrapperType result;
                if (value.get() == nullptr) {
                    return result;
                }
                for (auto const &element: *value.get()) {
                    result.push_back(traits<T>::do_unwrapper(element, policy));
                }
                return result;
            }

            static WrapperType do_wrapper(UnwrapperType const &value) {
                WrapperType result = WrapperType::createShared();
                for (auto const &element: value) {
                    result->push_back(traits<T>::do_wrapper(element));
                }
                return result;
            }
        };

        /**
         * @brief oatpp::UnorderedSet<T> -> std::unordered_set<递归解包后的元素类型>
         *        注意：std::unordered_set 无 push_back，只能 insert（历史缺陷 CF03）；
         *        null 容器 -> 空 set（内建约定）
         */
        template<typename T>
        struct traits<UnorderedSet<T> > : traits_base<type_category::container> {
            using WrapperType = UnorderedSet<T>;

            using UnwrapperType = std::unordered_set<typename traits<T>::UnwrapperType>;

            template<typename Policy = null_to_throw>
            static UnwrapperType do_unwrapper(UnorderedSet<T> const &value, Policy const &policy = {}) {
                UnwrapperType result;
                if (value.get() == nullptr) {
                    return result;
                }
                for (auto const &element: *value.get()) {
                    result.insert(traits<T>::do_unwrapper(element, policy));
                }
                return result;
            }

            static WrapperType do_wrapper(UnwrapperType const &value) {
                WrapperType result = WrapperType::createShared();
                for (auto const &element: value) {
                    result->insert(traits<T>::do_wrapper(element));
                }
                return result;
            }
        };

        /**
         * @brief oatpp::PairList<K, V>（即 DTO 中的 Fields）
         *        -> std::list<std::pair<递归解包后的 K, 递归解包后的 V> >
         *        注意：pair 的两个成员都递归解包（历史缺陷 CF06 的类型层问题）；
         *        std::list 无 reserve，只能 emplace_back；null 容器 -> 空 list（内建约定）
         */
        template<typename K, typename V>
        struct traits<PairList<K, V> > : traits_base<type_category::container> {
            using WrapperType = PairList<K, V>;

            using UnwrapperType = std::list<std::pair<
                typename traits<K>::UnwrapperType,
                typename traits<V>::UnwrapperType> >;

            template<typename Policy = null_to_throw>
            static UnwrapperType do_unwrapper(PairList<K, V> const &value, Policy const &policy = {}) {
                UnwrapperType result;
                if (value.get() == nullptr) {
                    return result;
                }
                for (auto const &entry: *value.get()) {
                    result.emplace_back(traits<K>::do_unwrapper(entry.first, policy),
                                        traits<V>::do_unwrapper(entry.second, policy));
                }
                return result;
            }

            static WrapperType do_wrapper(UnwrapperType const &value) {
                WrapperType result = WrapperType::createShared();
                for (auto const &entry: value) {
                    result->emplace_back(traits<K>::do_wrapper(entry.first),
                                         traits<V>::do_wrapper(entry.second));
                }
                return result;
            }
        };

        /**
         * @brief oatpp::UnorderedMap<K, V>（即 DTO 中的 UnorderedFields）
         *        -> std::unordered_map<递归解包后的 K, 递归解包后的 V>
         *        key 与 value 均递归解包（历史缺陷 CF01 的类型层问题）；
         *        null 容器 -> 空 map（内建约定）
         */
        template<typename K, typename V>
        struct traits<UnorderedMap<K, V> > : traits_base<type_category::container> {
            using WrapperType = UnorderedMap<K, V>;

            using UnwrapperType = std::unordered_map<
                typename traits<K>::UnwrapperType,
                typename traits<V>::UnwrapperType>;

            template<typename Policy = null_to_throw>
            static UnwrapperType do_unwrapper(UnorderedMap<K, V> const &value, Policy const &policy = {}) {
                UnwrapperType result;
                if (value.get() == nullptr) {
                    return result;
                }
                for (auto const &entry: *value.get()) {
                    result.emplace(traits<K>::do_unwrapper(entry.first, policy),
                                   traits<V>::do_unwrapper(entry.second, policy));
                }
                return result;
            }

            static WrapperType do_wrapper(UnwrapperType const &value) {
                WrapperType result = WrapperType::createShared();
                for (auto const &entry: value) {
                    result->emplace(traits<K>::do_wrapper(entry.first),
                                    traits<V>::do_wrapper(entry.second));
                }
                return result;
            }
        };

        /**
         * @brief DTO 定制点的辅助基类：用户全特化 traits<DTOWrapper<MyDto>> 时继承它，
         *        只需再提供 do_unwrapper（WrapperType/UnwrapperType/category 已填好）；
         *        如需反向包装（StructT -> DTO），再自行提供 do_wrapper 即可。
         *        DTO 内部逐字段的 null 语义由特化作者全权负责（策略不穿越用户代码）
         * @tparam Dto 用户的 DTO 类型
         * @tparam StructT 用户自定义的解包目标类型（长什么样完全由用户决定）
         */
        template<typename Dto, typename StructT>
        struct dto_traits_base : traits_base<type_category::object> {
            using WrapperType = data::mapping::type::DTOWrapper<Dto>;

            using UnwrapperType = StructT;
        };

        /**
         * @brief DTOWrapper<T> 默认没有解包目标类型，给出编译期诊断（ISSUE-DTO 修复方向 a）。
         *        用户对自己的 DTO 写 traits 的全特化（建议继承 dto_traits_base）即可覆盖本诊断；
         *        全特化后 Vector<Object<Dto>> 等嵌套容器的解包自动可用。
         */
        template<typename T>
        struct traits<data::mapping::type::DTOWrapper<T> > {
            static_assert(sizeof(T) != sizeof(T),
                          "traits<DTOWrapper<T>>: DTO 无默认解包目标类型，"
                          "请为你的 DTO 全特化 oatpp::meta_operation::traits（可继承 dto_traits_base）");
        };
    }
}

#endif //OATPP_HELPER_HPP
