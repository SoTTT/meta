#ifndef META_OPERATION_HPP
#define META_OPERATION_HPP

#include <algorithm>
#include <type_traits>
#include <absl/meta/type_traits.h>
#include <absl/utility/utility.h>
#include "traits.hpp"

namespace meta_operation {
    /**
     * @brief 这个命名空间提供其它工具所需要的类型萃取器和辅助模板
     */
    namespace helper {
        template<typename Iterator>
        struct container_view {
            using value_type = typename type_traits::iterator_traits_adaptor<Iterator>::value_type;
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

        template<typename Container, typename Transformer>
        void flat_foreach(Container &container, Transformer &&transformer, std::true_type) {
            std::forward<Transformer>(transformer)(container);
        }


        template<typename Container, typename Transformer>
        void flat_foreach(Container &container, Transformer &&transformer, std::false_type) {
            for (auto &item: container) {
                impl::flat_foreach(item, std::forward<Transformer>(transformer),
                                   type_traits::is_container_and_element_type_is_not_container<absl::remove_cvref_t<
                                       Container> >{}
                );
            }
        }

        template<typename InputIt, typename OutputIt, typename Transformer>
        void flat_transform(InputIt &begin, InputIt &end, OutputIt &dest, Transformer &transformer, std::true_type) {
            std::transform(begin, end, dest, transformer);
        }

        template<typename InputIt, typename OutputIt, typename Transformer>
        void flat_transform(InputIt &begin, InputIt &end, OutputIt &dest, Transformer &transformer, std::false_type) {
            std::transform(begin, end, dest,
                           [&transformer](typename InputIt::value_type item) {
                               absl::remove_cvref_t<decltype(item)> inner;
                               inner.reserve(item.size());
                               impl::flat_transform(item, std::back_inserter(inner), transformer,
                                                    type_traits::is_container_and_element_type_is_not_container<
                                                        absl::remove_cvref_t<decltype(item)> >{});
                               return inner;
                           });
        }

        template<class Container, typename OutputIt, typename Transformer>
        void flat_transform(Container &container, OutputIt destIt, Transformer transformer,
                            std::true_type) {
            std::transform(std::begin(container),
                           std::end(container),
                           destIt,
                           transformer);
        }

        template<class Container, typename OutputIt, typename Transformer>
        void flat_transform(Container &container, OutputIt destIt, Transformer transformer,
                            std::false_type) {
            std::transform(std::begin(container), std::end(container), destIt,
                           [&transformer](typename Container::value_type &item) {
                               absl::remove_cvref_t<decltype(item)> inner;
                               inner.reserve(item.size());
                               impl::flat_transform(item, std::back_inserter(inner), transformer,
                                                    type_traits::is_container_and_element_type_is_not_container<
                                                        absl::remove_cvref_t<decltype(item)> >{});
                               return inner;
                           });
            // for (auto&innerContainer: container) {
            //     typename helper::remove_cv_ref<decltype(innerContainer)>::type inner;
            //     inner.reserve(innerContainer.size());
            //     impl::flat_transform(innerContainer,
            //                          std::back_inserter(inner),
            //                          std::forward<Transformer>(transformer),
            //                          helper::is_container_and_element_type_is_not_container<typename
            //                              helper::remove_cv_ref<decltype(innerContainer
            //                              )>::type>{});
            //     *destIt++ = std::move(inner);
            // }
        }

        template<class Container, typename DestContainer, typename Transformer>
        void flat_foreach_nest(Container &container, DestContainer &dest, Transformer &&transformer, std::true_type) {
            std::transform(std::begin(container), std::end(container), std::begin(dest),
                           std::forward<Transformer>(transformer));
        }

        template<class Container, typename DestContainer, typename Transformer>
        void flat_foreach_nest(Container &container, DestContainer &dest, Transformer &&transformer, std::false_type) {
            for (int i = 0; i < container.size(); i++) {
                flat_foreach_nest(container[i], dest[i], std::forward<Transformer>(transformer),
                                  type_traits::is_container_and_element_type_is_not_container<
                                      absl::remove_cvref_t<decltype(container[i])> >{});
            }
        }
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
                           type_traits::is_container_and_element_type_is_not_container<
                               absl::remove_cvref_t<Container> >{}
        );
    }

    template<typename Iterator, typename Transformer>
    void flat_foreach(Iterator begin, Iterator end, Transformer &&transformer) {
        helper::container_view<Iterator> view{begin, end};
        flat_foreach(view, std::forward<Transformer>(transformer));
    }

    /**
     * @brief 遍历container，并将容器中的每一个元素应用于一元操作transformer，并返回由每一个操作结果构成的容器，在遍历时将有多个维度的容器视为一维的
     * @param container 待变换的容器
     * @param transformer 用于执行变换的一元操作
     * @return 将一元操作作用于待变换容器的各个元素的结果按照待变换容器的维度和次序构成的容器
     */
    template<class Container, typename Transformer>
    auto flat_transform(Container &container,
                        Transformer &&transformer) -> absl::remove_cvref_t<Container> {
        absl::remove_cvref_t<Container> dest(container.size());
        impl::flat_transform(container,
                             dest.begin(),
                             std::forward<Transformer>(transformer),
                             type_traits::is_container_and_element_type_is_not_container<absl::remove_cvref_t<Container> >
                             {});
        return dest;
    }

    /**
     * @brief 遍历由一组迭代器对指出的范围，将容器中的每一个元素应用于一元操作transformer，并置于由dest作为起始位置的范围的对应位置，在遍历时将有多个维度的容器视为一维的
     * @param begin 一个迭代器，用于指出待变换范围的开始
     * @param end 一个迭代器，用于指出待变换范围的尾部
     * @param dest 一个输出迭代器，用于指出变换的目标范围
     * @param transformer 用于执行变换的一元操作
     */
    template<typename InputIt, typename OutputIt, typename Transformer>
    void flat_transform(InputIt begin, InputIt end, OutputIt dest, Transformer &&transformer) {
        helper::container_view<InputIt> view{begin, end};
        impl::flat_transform(view, dest, std::forward<Transformer>(transformer),
                             type_traits::is_container_and_element_type_is_not_container<helper::container_view<OutputIt> >
                             {});
    }

    /**
     * @brief 遍历由一组迭代器对指出的范围，将容器中的每一个元素应用于一元操作transformer，并置于由dest作为起始位置的范围的对应位置，在遍历时将有多个维度的容器视为一维的
     * @param begin 一个迭代器，用于指出待变换范围的开始
     * @param end 一个迭代器，用于指出待变换范围的尾部
     * @param dest 一个尾部插入，用于指出变换的目标范围
     * @param transformer 用于执行变换的一元操作
     * @pre
     * 若使用插入迭代器适配器时，应保证最外层容器的长度为0，对于其他输入迭代器，应保证最外层容器的长度符合源范围长度，对于内层容器，
     * 最好的情况下应保持其为空，若内层容器非空不会引发错误，但会导致额外的内存释放和元素析构
     * @note
     * 在使用std::transform时，经常使用std::back_insert_iterator以省去调整容器大小的操作。
     * 在flat_transform中，实现需要根据OutputIt的类型特征判断是否应该终止递归模板展开；但是，受标准库实现的限制，在C++20前，对于一个容器类型
     * Container，任意的迭代器适配器IteratorAdaptor<Container>都无法通过std::iterator_traits正确地获取其类型特征，对于元素类型，
     * std::iterator_traits<IteratorAdaptor<Container>>::value_type始终为void，这导致
     * @code
     * meta_operation::flat_transform(c.begin(), c.end(), std::back_inserter(dest), transformer);
     * @endcode
     * 这样的代码实际上无法通过编译，为了使用法尽可能与标准库算法保持一致，该实现为std::back_insert_iterator提供了支持，
     * 使上述代码可以正常通过编译，对于其它迭代器适配器则暂未提供类似的支持；
     */
    template<typename InputIt, typename DestContainer, typename Transformer>
    void flat_transform(InputIt &begin, InputIt &end, std::back_insert_iterator<DestContainer> &dest,
                        Transformer &transformer) {
        // helper::container_view<InputIt> view{begin, end};
        // impl::flat_transform(view, dest, transformer,
        //                      helper::is_container_and_element_type_is_not_container<helper::container_view<
        //                          std::back_insert_iterator<DestContainer>>>
        //                      {});
        impl::flat_transform(begin, end, dest, transformer,
                             type_traits::is_container_and_element_type_is_not_container<helper::container_view<
                                 std::back_insert_iterator<DestContainer> > >
                             {});
    }

    /**
     * @brief 遍历由一组迭代器对指出的范围，将容器中的每一个元素应用于一元操作transformer，并置于由dest作为起始位置的范围的对应位置，在遍历时将有多个维度的容器视为一维的
     * @param container 待变换的容器
     * @param dest 一个输出迭代器，用于指出变换的目标范围
     * @param transformer 用于执行变换的一元操作
     * @pre 实现要求container和dest的维度和每个维度上的长度是一致的，也就是说，container是一个预先调整长度的向量、矩阵或张量
     */
    template<typename Container, typename DestContainer, typename Transformer>
    void flat_transform(Container &container, DestContainer &dest, Transformer &&transformer) {
        impl::flat_foreach_nest(container, dest, std::forward<Transformer>(transformer),
                                type_traits::is_container_and_element_type_is_not_container<absl::remove_cvref_t<Container> >
                                {});
    }
}

#endif //META_OPERATION_HPP
