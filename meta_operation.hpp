#ifndef META_OPERATION_HPP
#define META_OPERATION_HPP

#include <algorithm>
#include <type_traits>

namespace meta_operation {
    /**
     * @brief 这个命名空间提供其它工具所需要的类型萃取器和辅助模板
     */
    namespace helper {
        template<typename... Args>
        using void_t = void;
        /**
         * @brief 移除给定的类型上的引用、只读和易变性修饰
         */
        template<typename T>
        struct remove_cv_ref {
            using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
        };

        template<typename T>
        struct element_type {
            using type = typename T::value_type;
        };

        /**
         * @brief 鉴别一个类型是否满足容器的语法要求
         * @tparam T 要鉴别的类型
         * @memberof value 鉴别的结果
         */
        template<typename T, typename = void>
        struct is_container : std::false_type {
        };

        template<typename T, size_t N>
        struct is_container<T[N]> : std::true_type {
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
                                             && !is_container<typename element_type<T>::type>::value> {
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

        template<typename Iterator>
        struct container_view {
            using value_type = typename iterator_traits_adaptor<Iterator>::value_type;
            using iterator = Iterator;

            Iterator &_begin;
            Iterator &_end;

            [[nodiscard]] Iterator begin() const { return _begin; }
            [[nodiscard]] Iterator end() const { return _end; }

            container_view(Iterator &begin, Iterator &end) : _begin(begin), _end(end) {
            }
        };
    }

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


        template<typename Container, typename Transformer>
        void flat_foreach(Container &container, Transformer &&transformer, std::true_type) {
            std::forward<Transformer>(transformer)(container);
        }


        template<typename Container, typename Transformer>
        void flat_foreach(Container &container, Transformer &&transformer, std::false_type) {
            for (auto &item: container) {
                impl::flat_foreach(item, std::forward<Transformer>(transformer),
                                   helper::is_container_and_element_type_is_not_container<typename helper::remove_cv_ref
                                       <Container>::type>{}
                );
            }
        }

        template<class Container, typename OutputIt, typename Transformer>
        void flat_transform(Container &container, OutputIt destIt, Transformer &&transformer,
                            std::true_type) {
            std::transform(std::begin(container),
                           std::end(container),
                           destIt,
                           std::forward<Transformer>(transformer));
        }

        template<class Container, typename OutputIt, typename Transformer>
        void flat_transform(Container &container, OutputIt destIt, Transformer &&transformer,
                            std::false_type) {
            for (auto &innerContainer: container) {
                typename helper::remove_cv_ref<decltype(innerContainer)>::type inner(innerContainer.size());
                impl::flat_transform(innerContainer,
                                     inner.begin(),
                                     std::forward<Transformer>(transformer),
                                     helper::is_container_and_element_type_is_not_container<typename
                                         helper::remove_cv_ref<decltype(innerContainer
                                         )>::type>{});
                *destIt++ = std::move(inner);
            }
        }
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
     * @brief 遍历一个容器并将容器的各个元素应用到transformer，在遍历时将拥有多个维度的容器视为一维的
     * @tparam Container 需要遍历的容器类型
     * @tparam Transformer 要应用的容器上的可调用对象或函数类型
     * @param container 需要遍历的容器
     * @param transformer 要应用的容器上的可调用对象或函数
     */
    template<typename Container, typename Transformer>
    void flat_foreach(Container &container, Transformer &&transformer) {
        impl::flat_foreach(container,
                           std::forward<Transformer>(transformer),
                           helper::is_container_and_element_type_is_not_container<
                               typename helper::remove_cv_ref<Container>::type>{}
        );
    }

    template<typename Iterator, typename Transformer>
    void flat_foreach(Iterator begin, Iterator end, Transformer &&transformer) {
        helper::container_view<Iterator> view{begin, end};
        flat_foreach(view, std::forward<Transformer>(transformer));
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
    struct replace_other_container_type_with<T, Container, typename std::enable_if<helper::is_container<
                T>::value>::type> {
        using type = Container<typename replace_other_container_type_with<typename T::value_type, Container>::type>;
    };


    template<typename T, template <typename...> class Container>
    using replace_other_container_type_with_t = typename replace_other_container_type_with<T, Container>::type;


    /**
     * @brief 遍历container，并将容器中的每一个元素应用于一元操作transformer，并返回由每一个操作结果构成的容器，在遍历时将有多个维度的容器视为一维的
     * @param container 待变换的容器
     * @param transformer 用于执行变换的一元操作
     * @return 将一元操作作用于待变换容器的各个元素的结果按照待变换容器的维度和次序构成的容器
     */
    template<class Container, typename Transformer>
    auto flat_transform(Container &container, Transformer &&transformer) {
        typename helper::remove_cv_ref<Container>::type dest(container.size());
        impl::flat_transform(container,
                             dest.begin(),
                             std::forward<Transformer>(transformer),
                             helper::is_container_and_element_type_is_not_container<typename helper::remove_cv_ref<
                                 Container>::type>{});
        return dest;
    }

    /**
     * @brief 遍历由一组迭代器对指出的范围，将容器中的每一个元素应用于一元操作transformer，并置于由dest作为起始位置的范围的对应位置，在遍历时将有多个维度的容器视为一维的
     * @param begin 一个迭代器，用于指出待变换范围的开始
     * @param end 一个迭代器，用于指出待变换范围的尾部
     * @param dest 一个输出迭代器，用于指出变换的目标范围
     * @param transformer 用于执行变换的一元操作
     * @note
     * 在使用std::transform时，经常使用std::back_insert_iterator以省去调整容器大小的操作。
     * 在flat_transform中，实现需要根据OutputIt的类型特征判断是否应该终止递归模板展开；但是，受标准库实现的限制，在C++20前，对于一个容器类型
     * Container，任意的迭代器适配器IteratorAdaptor<Container>都无法通过std::iterator_traits正确地获取其类型特征，对于元素类型，
     * std::iterator_traits<IteratorAdaptor<Container>>::value_type始终为void，这导致
     * meta_operation::flat_transform(c.begin(), c.end(), std::back_inserter(dest), transformer) 这样的代码实际上无法
     * 通过编译，为了使用法尽可能与标准库算法保持一致，实现对std::back_insert_iterator进行了定制，使上述代码可以正常通过编译，对于其它迭代器适
     * 配器则暂未提供类似的支持；
     */
    template<typename InputIt, typename OutputIt, typename Transformer>
    void flat_transform(InputIt begin, InputIt end, OutputIt dest, Transformer &&transformer) {
        helper::container_view<InputIt> view{begin, end};
        impl::flat_transform(view, dest, std::forward<Transformer>(transformer),
                             helper::is_container_and_element_type_is_not_container<helper::container_view<OutputIt> >
                             {});
    }
}

#endif //META_OPERATION_HPP
