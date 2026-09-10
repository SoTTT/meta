# oatpp-meta

**中文** | [English](i18n/README.en.md)

单头文件 **C++11** 模板元编程库（CMake target：`oatpp_meta`，`INTERFACE`），核心能力是在
std 类型与 oatpp 包装类型（`oatpp::Vector<T>`、`oatpp::String`、`oatpp::Int32`、`DTOWrapper`
等）之间做**类型层深解包 / 深包装与运行时转换**。

## 核心接口：`oatpp::meta::traits<T>`

按 oatpp 类型族提供全特化，每个特化以下成员：

| 成员 | 说明 |
| --- | --- |
| `WrapperType` | oatpp 包装类型自身 |
| `UnwrapperType` | 深解包后的 std 目标类型（嵌套容器递归展开） |
| `do_unwrapper(value, Policy)` | 运行时解包；Policy 决定标量 null 语义，沿递归逐层传递 |
| `do_wrapper(value)` | 运行时反向包装，把 `UnwrapperType` 包装成 `WrapperType`；恒产生非 null 包装 |

对于以下oatpp类型，本库提供了特化：

- 非 oatpp 类型：原样穿透；
- `oatpp::Int8...Float64` 等 `Primitive<T, Clazz>`：数值原语；
- `oatpp::String` / `oatpp::Boolean` / `EnumObjectWrapper`：scalar 叶子；
- 容器类型（`oatpp::Vector/List/UnorderedSet`、`PairList`、`UnorderedMap`）：元素与键值递归解包；
- `oatpp::Void` / `oatpp::Any`：无法静态解包，按原样保留；
- DTO（`oatpp::Object<T>`）：**用户定制点**（见下）。

类型族标志由公共基类 `traits_base<type_category>` 派生：全部 `is_xxx` 常量由互斥的
`type_category`（passthrough / primitive / scalar / container / object / opaque）自动导出，
非法组合不可表达。

### 用法示例

```cpp
#include <meta.hpp>
#include <oatpp/core/Types.hpp>

using oatpp::meta::traits;

oatpp::String s("hello");
std::string v = traits<oatpp::String>::do_unwrapper(s);          // "hello"

auto vec = oatpp::Vector<oatpp::Int32>::createShared();
vec->push_back(oatpp::Int32(1));
vec->push_back(oatpp::Int32(2));
std::vector<std::int32_t> r = traits<oatpp::Vector<oatpp::Int32>>::do_unwrapper(vec); // {1, 2}

std::vector<std::int32_t> back = {7, 8};
oatpp::Vector<oatpp::Int32> w = traits<oatpp::Vector<oatpp::Int32>>::do_wrapper(back); // 非 null 包装
```

## null 对象处理

oatpp的对象本质上是带有运行时类型信息的指针，所以oatpp对象都是可空的，为了对空对象进行自定义处理，oatpp-meta为不同的类型提供了策略；

1. 容器转换为空容器；
2. 空标量类型的处理由**Policy**决定：
   - 默认 `null_to_throw`：视为数据契约违反，抛 `null_unwrap_error`；
   - 宽容需显式传 `null_to_default{}`：映射为默认值（会丢失 null 与零值的区分）；
   - 自定义策略只需提供 `template<typename U> static U on_null_scalar()`；
3. 值为null的DTO对象的处理由用户的自定义转换代码决定；

```cpp
oatpp::Int32 n;                                        // null 标量
traits<oatpp::Int32>::do_unwrapper(n);                 // 抛 null_unwrap_error
traits<oatpp::Int32>::do_unwrapper(n, oatpp::meta::null_to_default{}); // 0
```

### 按类型分派：`policy::combine`

同一次解包中为不同类型的标量指定不同策略，按声明顺序**首个命中生效**：

```cpp
using my_policy = oatpp::meta::policy::combine<
    oatpp::meta::policy::for_type<std::int32_t, oatpp::meta::null_to_default>,
    oatpp::meta::policy::for_type<std::string, oatpp::meta::null_to_default>,
    oatpp::meta::policy::otherwise<oatpp::meta::null_to_throw>
>;

auto result = traits<SomeOatppType>::do_unwrapper(value, my_policy{});
```

## DTO 定制点

为了对DTO对象进行统一转换，oatpp-meta提供了定制点，用户可以通过继承辅助基类 `dto_traits_base<MyDto, MyStruct>`、特化`oatpp::meta::traits`并实现 `do_unwrapper`（及可选的 `do_wrapper`）为oatpp DTO提供支持，一个例子如下：

```cpp
namespace oatpp { namespace meta {
template<>
struct traits<oatpp::Object<TestDto>> : dto_traits_base<TestDto, TestStruct> {
    template<typename Policy = null_to_throw>
    static UnwrapperType do_unwrapper(oatpp::Object<TestDto> const &value, Policy const & = {}) {
        TestStruct result{};
        if (value) {
            result.id = traits<oatpp::Int32>::do_unwrapper(value->id);
            // ... 逐字段解包（null 语义由本特化作者决定）
        }
        return result;
    }
};
}}
```

##  oatpp 依赖

本项目仿照官方 oatpp 组件（如 oatpp-swagger）的做法，在配置阶段用 `OATPP_MODULES_LOCATION`
变量选择 oatpp 依赖的来源，取值如下：

- `AUTO`（默认）：按顺序自动探测。本机已安装 oatpp 1.3.0 及以上就用它，否则回退到 `EXTERNAL`。
- `INSTALLED`：使用 `find_package` 找到的已安装 oatpp（可另用 `oatpp_DIR` 指向自定义安装位置）。
- `EXTERNAL`：在构建阶段从 GitHub 下载 oatpp 并本地编译。默认拉取 `origin/master`，
  可用 `OATPP_GIT_TAG` 固定到某个版本或分支。
- `CUSTOM`：使用本地已有的 oatpp 头文件与库。`OATPP_DIR_SRC` 指定含 oatpp 头文件的目录
  （形如 `<oatpp 源码>/src`），`OATPP_DIR_LIB` 指定含 `liboatpp` 的目录。

常用配置命令：

```bash
cmake ..                                                        # 默认 AUTO
cmake -DOATPP_MODULES_LOCATION=EXTERNAL ..                      # 显式远程拉取并构建
cmake -DOATPP_MODULES_LOCATION=EXTERNAL -DOATPP_GIT_TAG=1.3.0 ..  # 固定远程版本
cmake -DOATPP_MODULES_LOCATION=CUSTOM \
      -DOATPP_DIR_SRC=<oatpp 源码>/src -DOATPP_DIR_LIB=<oatpp 库目录> ..  # 本地源码/库
```

取值不合法、`INSTALLED` 找不到 oatpp、`CUSTOM` 缺少或填错目录时，配置会给出指引式报错。
本项目作为子目录（`add_subdirectory`）被其他项目引用时，如果调用方已经提供了 oatpp 目标，
会直接复用，不再查找或下载。

## 构建与测试

```bash
mkdir -p build && cd build          # 或用 CLion 的 cmake-build-debug/
cmake ..                            # 默认 AUTO：本机无已安装 oatpp 时，构建阶段会联网拉取并编译
cmake --build .                     # 测试可执行文件：build/test/oatpp_meta_test
./test/oatpp_meta_test              # 全部用例（当前 22 用例 / 120 断言全绿）
./test/oatpp_meta_test "[null]"     # 按 tag 过滤；现有 tag：[traits] [null] [dto] [policy]
```

- 语言标准：C++11（`CMAKE_CXX_STANDARD 11`），oatpp / Catch2 亦以 C++11 编译。
- 作为库使用时只需引入 `src/meta.hpp`；库本体只依赖 oatpp，具体来源见上文。
- 开发环境：macOS / Apple Clang。
