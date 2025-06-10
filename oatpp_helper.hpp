//
// Created by H1773 on 25-6-7.
//

#ifndef OATPP_HELPER_HPP
#define OATPP_HELPER_HPP

#include <type_traits>
#include <oatpp/core/Types.hpp>

#include "template_helper.hpp"

namespace meta_operation {
    namespace type_traits {
        namespace oatpp {
            template<typename T, typename = void>
            struct is_oatpp_wrapper : std::false_type {
            };

            template<typename T>
            struct is_oatpp_wrapper<T,
                        absl::enable_if_t<std::is_void<absl::void_t<typename T::ObjectType> >::value> >
                    : std::is_base_of<::oatpp::ObjectWrapper<
                            typename T::ObjectType,
                            typename template_helper::get_last_arg_from_template<T>::type>,
                        T> {
            };

            template<typename T, typename = void>
            struct is_oatpp_primitive_wrapper : std::false_type {
            };

            template<typename T>
            struct is_oatpp_primitive_wrapper<T, absl::enable_if_t<is_oatpp_wrapper<T>::value> >
                    : std::is_same<::oatpp::data::mapping::type::Primitive<
                            typename T::ObjectType,
                            typename template_helper::get_last_arg_from_template<T>::type>,
                        T> {
            };

            template<typename T, typename = void>
            struct is_oatpp_container_wrapper : std::false_type {
            };

            template<typename T>
            struct is_oatpp_container_wrapper<T, absl::enable_if_t<
                        is_oatpp_wrapper<T>::value &&
                        is_container<typename T::TemplateObjectType>::value> >
                    : std::true_type {
            };

            template<typename T, typename = void>
            struct value_type_is_oatpp_container_warper : std::false_type {
            };

            template<typename T>
            struct value_type_is_oatpp_container_warper<T, absl::enable_if_t<
                        is_container<T>::value &&
                        is_oatpp_container_wrapper<typename T::value_type>::value> >
                    : std::true_type {
            };

            template<typename T>
            struct is_oatpp_container_wrapper_clazz : std::false_type {
            };

            template<typename T>
            struct is_oatpp_container_wrapper_clazz<::oatpp::data::mapping::type::__class::Vector<T> >
                    : std::true_type {
            };

            template<typename T, typename = void>
            struct unwrapper {
                using type = T;
            };

            template<typename T>
            struct unwrapper<T, absl::enable_if_t<is_oatpp_container_wrapper<T>::value> > {
                using type = typename template_helper::replace_type<
                    typename T::TemplateObjectType,
                    typename unwrapper<typename T::TemplateObjectType::value_type>::type
                >::type;
            };

            template<typename T>
            struct unwrapper<T, absl::enable_if_t<
                        is_oatpp_wrapper<T>::value &&
                        is_oatpp_primitive_wrapper<T>::value> > {
                using type = typename T::ObjectType;
            };

            template<typename T>
            struct is_oatpp_container_wrapper_clazz<::oatpp::data::mapping::type::__class::List<T> >
                    : std::true_type {
            };

            template<typename T>
            struct is_oatpp_container_wrapper_clazz<::oatpp::data::mapping::type::__class::UnorderedSet<T> >
                    : std::true_type {
            };

            template<typename K, typename V>
            struct is_oatpp_container_wrapper_clazz<::oatpp::data::mapping::type::__class::UnorderedMap<K, V> >
                    : std::true_type {
            };
        }
    }
}

#endif //OATPP_HELPER_HPP
