//
// Created by H1773 on 25-6-8.
//

#ifndef TEMPLATE_HELPER_HPP
#define TEMPLATE_HELPER_HPP

#include <type_traits>

namespace meta_operation {
    namespace template_helper {
        namespace impl {
            template<int Ns, typename T, typename... Other>
            struct get_Ns_arg {
                using type = typename get_Ns_arg<Ns - 1, Other...>::type;
            };

            template<typename T, typename... Other>
            struct get_Ns_arg<0, T, Other...> {
                using type = T;
            };

            template<typename T, typename N>
            struct match_template_arguments_helper;

            template<template<typename...> class T, typename N, typename... E>
            struct match_template_arguments_helper<T<E...>, N> {
                template<typename... Args>
                using type = T<Args...>;
            };

            template<typename... Types>
            struct wrapper_type_list {
            };
        }

        template<int Ns, typename... Other>
        struct get_Ns_arg {
            using type = typename impl::get_Ns_arg<Ns, Other...>::type;
        };

        template<typename... Others>
        struct get_first_arg {
            using type = typename get_Ns_arg<0, Others...>::type;
        };

        template<typename... Others>
        struct get_last_arg {
            using type = typename get_Ns_arg<sizeof...(Others) - 1, Others...>::type;
        };

        template<int Ns, typename T>
        struct get_Ns_arg_from_template;

        /**
         * @brief 获取一个模板实例化类型的第Ns个参数的类型
         * @tparam T 一个模板参数类型
         * @tparam Ns 一个整数索引，指出想要获取的类型在类型列表中的位置
         * @tparam E 边长参数包，用于接受变长模板模板参数的参数列表
         * @memberof type 模板实例化类型的第Ns个模板参数
         */
        template<template<class...> class T, int Ns, typename... E>
        struct get_Ns_arg_from_template<Ns, T<E...> > {
            using type = typename get_Ns_arg<Ns, E...>::type;
        };

        template<typename T>
        struct get_last_arg_from_template;

        template<template<class...> class T, typename... E>
        struct get_last_arg_from_template<T<E...> > {
            using type = typename get_last_arg<E...>::type;
        };

        /**
         * @brief 给出一个模板实例化类型TMP，并以给出的变长类型参数包N替换它的模板实参
         * @tparam TMP 一个模板实例化类型，如std::vector<int>
         * @tparam N 一个变长类型参数包
         * @memberof type 模板实例化类型的第Ns个模板参数
         */
        template<typename TMP, typename... N>
        struct replace_type {
            using type = typename impl::match_template_arguments_helper
            <TMP, impl::wrapper_type_list<N...> >::template type<N...>;
        };

        template<typename T>
        struct get_result_type;

        template<typename ResultType, typename... Args>
        struct get_result_type<ResultType(Args...)> {
            using type = ResultType;
        };

        template<typename T>
        struct rank {
            static constexpr int n = 0;
        };

        template<template<class...> class TMP, typename... TemplateArgs>
        struct rank<TMP<TemplateArgs...> > {
            static constexpr int n = rank<typename get_first_arg<TemplateArgs...>::type>::n + 1;
        };

        template<typename T>
        constexpr int get_rank(T &&) {
            return rank<typename std::remove_reference<typename std::remove_cv<T>::type>::type>::n;
        }

        /**
         * @brief 将类型的外层模板类型转换为给定的模板类型，并保持类型T的维度不变
         * @tparam T 需要转换的原始的原始模板实例化对象类型
         * @tparam Container 转换目标的模板
         * @tparam Placement 为模板特化实现保留的占位符
         * @memberof type 转换得到的类型
         */
        template<typename T, template <typename...> class Container, typename Placement=void>
        struct replace_other_container_type_with {
            using type = T;
        };

        template<typename T, template <typename...> class Container>
        struct replace_other_container_type_with<T, Container, typename std::enable_if<type_traits::is_container<
                    T>::value>::type> {
            using type = Container<typename replace_other_container_type_with<typename T::value_type, Container>::type>;
        };

        template<typename T, template <typename...> class Container>
        using replace_other_container_type_with_t = typename replace_other_container_type_with<T, Container>::type;
    }
}

#endif //TEMPLATE_HELPER_HPP
