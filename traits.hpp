//
// Created by H1773 on 25-6-7.
//

#ifndef TRAITS_HPP
#define TRAITS_HPP

#include <type_traits>
#include <string>

namespace meta_operation {
    namespace type_traits {
        template<typename... Args>
        using void_t = void;

        /**
         * @brief 鉴别一个类型是否满足容器的语法要求
         * @tparam T 要鉴别的类型
         * @memberof value 鉴别的结果
         */
        template<typename T, typename = void>
        struct is_container : std::false_type {
        };

        template<typename char_t>
        struct is_container<std::basic_string<char_t> > : std::false_type {
        };

        template<typename T>
        struct is_container<T, void_t<
                    typename T::value_type,
                    typename T::iterator,
                    decltype(std::declval<T>().begin()),
                    decltype(std::declval<T>().end())
                > > : std::true_type {
        };

        /**
         * @brief 鉴别一个类型是否是由模板实例化而来
         * @tparam T 需要鉴别的类型
         */
        template<typename T>
        struct is_template_instance : std::false_type {
        };

        template<template <typename...> class C, typename... Args>
        struct is_template_instance<C<Args...> > : std::true_type {
        };

        /**
         * @brief 鉴别一个类型是否本身是容器但其元素类型不是容器
         * @tparam T 要鉴别的类型
         */
        template<typename T>
        struct is_container_and_element_type_is_not_container :
                std::integral_constant<bool, is_container<T>::value
                                             && !is_container<typename T::value_type>::value> {
        };

        /**
         * @brief 鉴别一个类型T是否由一个模板TMP实例化而来
         * @tparam T 鉴别的目标类型
         * @tparam TMP 鉴别的目标模板
         */
        template<typename T, template <typename...> class TMP>
        struct is_specialization_of : std::false_type {
        };

        template<template <typename...> class TMP, typename... Args>
        struct is_specialization_of<TMP<Args...>, TMP> : std::true_type {
        };

        template<typename Iterator>
        struct iterator_traits_adaptor : std::iterator_traits<Iterator> {
        };

        template<typename Container>
        struct iterator_traits_adaptor<std::back_insert_iterator<Container> > :
                std::iterator_traits<typename Container::iterator> {
        };
    }
}

#endif //TRAITS_HPP
