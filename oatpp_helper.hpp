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
            struct is_oatpp_map_container_wrapper : std::false_type {
            };

            template<typename K, typename V>
            struct is_oatpp_map_container_wrapper<::oatpp::UnorderedMap<K, V> > : std::true_type {
            };

            template<typename T, typename = void>
            struct value_type_is_oatpp_container_wrapper : std::false_type {
            };

            template<typename T>
            struct value_type_is_oatpp_container_wrapper<T, absl::enable_if_t<
                        is_container<T>::value &&
                        is_oatpp_container_wrapper<typename T::value_type>::value> >
                    : std::true_type {
            };


            /**
             * @brief 获取oatpp包装类型对应的原始类型，对于嵌套的容器包装类型，会递归地依次将每层容器包装类型替换为对应的原始类型,
             * 对于非oatpp包装类型的类型，返回其自身
             * @tparam T 待处理的类型
             * @memberof type 解包装后的类型
             */
            template<typename T, typename = void>
            struct unwrapper {
                using type = T;
            };

            template<typename T>
            struct unwrapper<T, absl::enable_if_t<
                        is_oatpp_container_wrapper<T>::value &&
                        !is_oatpp_map_container_wrapper<T>::value> > {
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
            struct unwrapper<T, absl::enable_if_t<
                        is_oatpp_container_wrapper<T>::value
                        && is_oatpp_map_container_wrapper<T>::value> > {
                using type = typename template_helper::replace_type<
                    typename T::TemplateObjectType,
                    typename unwrapper<typename T::TemplateObjectType::key_type>::type,
                    typename unwrapper<typename T::TemplateObjectType::mapped_type>::type
                >::type;
            };

            template<typename T>
            struct unwrapper<T, absl::enable_if_t<std::is_same<T, ::oatpp::String>::value>> {
                using type = std::string;
            };

            namespace impl {

                template<typename WrapperContainerType, typename UnwrapperContainerType, typename = void>
                struct deep_unwrapper;


                template<typename WrapperContainerType, typename UnwrapperContainerType>
                struct deep_unwrapper<WrapperContainerType, UnwrapperContainerType,
                            absl::enable_if_t<!is_oatpp_map_container_wrapper<WrapperContainerType>::value> > {
                    void operator()(WrapperContainerType const &wrapperContainer,
                                    UnwrapperContainerType &unwrapperContainer, std::true_type) {
                        unwrapperContainer.reserve(wrapperContainer->size());
                        for (int i = 0; i < wrapperContainer->size(); ++i) {
                            unwrapperContainer.push_back(wrapperContainer[i]);
                        }
                    }

                    void operator()(WrapperContainerType const &wrapperContainer,
                                    UnwrapperContainerType &unwrapperContainer, std::false_type) {
                        unwrapperContainer.resize(wrapperContainer->size());
                        for (int i = 0; i < wrapperContainer->size(); ++i) {
                            deep_unwrapper<
                                absl::remove_cvref_t<decltype(wrapperContainer[i])>,
                                absl::remove_cvref_t<decltype(unwrapperContainer[i])>
                            >{}
                            (wrapperContainer[i],
                             unwrapperContainer[i],
                             is_container_and_element_type_is_not_container<typename UnwrapperContainerType::value_type>
                             {});
                        }
                    }
                };

                template<typename WrapperContainerType, typename UnwrapperContainerType>
                struct deep_unwrapper<WrapperContainerType, UnwrapperContainerType,
                            absl::enable_if_t<is_oatpp_map_container_wrapper<WrapperContainerType>::value> > {
                    void operator()(WrapperContainerType const &wrapperContainer,
                                    UnwrapperContainerType &unwrapperContainer, std::false_type) {
                        for (auto it = wrapperContainer->begin(); it != wrapperContainer->end(); ++it) {
                            unwrapperContainer[it->first] =
                                    typename unwrapper<absl::remove_cvref_t<decltype(it->second)> >::type{};
                            deep_unwrapper<
                                absl::remove_cvref_t<decltype(wrapperContainer[it->first])>,
                                absl::remove_cvref_t<decltype(unwrapperContainer[it->first])>
                            >{}
                            (wrapperContainer[it->first], unwrapperContainer[it->first],
                             is_container_and_element_type_is_not_container<typename
                                 UnwrapperContainerType::value_type>{});
                        }
                    }

                    void operator()(WrapperContainerType const &wrapperContainer,
                                    UnwrapperContainerType &unwrapperContainer, std::true_type) {
                        for (auto it = wrapperContainer->begin(); it != wrapperContainer->end(); ++it) {
                            unwrapperContainer[it->first] = it->second;
                        }
                    }
                };
            }

            template<typename T>
            auto deep_unwrapper(T const &container) -> typename unwrapper<T>::type {
                typename unwrapper<T>::type unwrapperContainer;
                impl::deep_unwrapper<T, typename unwrapper<T>::type>{}
                (container, unwrapperContainer,
                 is_container_and_element_type_is_not_container<typename unwrapper<
                     T>::type>{});
                return unwrapperContainer;
            }

            template<typename T>
            struct is_oatpp_container_wrapper_clazz : std::false_type {
            };

            template<typename T>
            struct is_oatpp_container_wrapper_clazz<::oatpp::data::mapping::type::__class::Vector<T> >
                    : std::true_type {
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
