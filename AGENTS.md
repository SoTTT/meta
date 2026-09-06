# AGENTS.md — oatpp-meta

## 项目概述

`oatpp-meta` 是一个**纯头文件 C++11 模板元编程库**（CMake target `oatpp_meta`，`INTERFACE`），
在 std 类型与 oatpp 包装类型（`oatpp::Vector<T>`、`oatpp::String`、`DTOWrapper` 等）之间做
**类型层深解包/深包装与运行时转换**。仅依赖 oatpp 1.3.0（CMake FetchContent 拉取，构建需联网）。
代码注释与提交信息为中文，保持一致。

## 核心接口（src/meta.hpp，命名空间 `oatpp::meta`）

- `traits<T>`：按 oatpp 类型族全特化的入口。成员：`WrapperType` / `UnwrapperType` /
  `do_unwrapper(value, Policy)` / `do_wrapper(value)`。
- `traits_base<C>`：公共基类，只需指定互斥的 `type_category`（passthrough / primitive /
  scalar / container / object / opaque），全部 `is_xxx` 标志由此派生，非法组合不可表达。
- 内建特化：passthrough（非 oatpp 类型原样穿透）；`Primitive<T,Clazz>`（数值）；
  `String` / `Boolean` / `EnumObjectWrapper`（scalar）；`Void` / `Any`（opaque）；
  `Vector` / `List` / `UnorderedSet` / `PairList` / `UnorderedMap`（container，元素/键值
  递归解包，`do_wrapper` 恒产生非 null 包装）。
- **DTO 是用户定制点**：`traits<DTOWrapper<T>>` 默认 `static_assert`（DTO 无默认解包目标，
  属设计行为而非缺陷）。用户对自己的 DTO 全特化 `traits`，推荐继承辅助基类
  `dto_traits_base<MyDto, MyStruct>`（WrapperType / UnwrapperType / category 已填好），
  自行实现 `do_unwrapper`（及可选的 `do_wrapper`）；特化后 `Vector<Object<Dto>>` 等嵌套
  解包自动可用。
- **null 语义三层模型**（meta.hpp 文件头注释有完整版）：
  1. 容器 null → 空容器（内建约定，不进 Policy）；
  2. 标量 null（任意深度，含容器内元素）→ Policy 决定：默认 `null_to_throw`（抛
     `null_unwrap_error`），宽容必须显式传 `null_to_default{}`；自定义策略只需提供
     `template<typename U> static U on_null_scalar()`。可按类型声明式分派：
     `policy::combine<policy::for_type<T,P>, ..., policy::otherwise<P>>`（声明顺序匹配，
     首个命中生效）；
  3. 嵌套对象逐字段可空性/null 处理由用户 traits 特化全权负责，策略不穿越用户代码。
- **null 判定统一用 `value.get() == nullptr`，禁止 `!value`**：`oatpp::Boolean` 的 `false`
  会被 `!` 误判为 null（历史 bug，库内与测试均已按此统一）。

## 目录结构

- `src/` — 库本体：`meta.hpp`（唯一头文件，无 .cpp，include guard `OATPP_META_HPP`）+
  `CMakeLists.txt`（INTERFACE target `oatpp_meta`）
- `test/` — `test.cpp`（Catch2 v2，含 `CATCH_CONFIG_MAIN`，22 个用例）+ `CMakeLists.txt`
  （FetchContent 拉 Catch2 v2.13.10；可执行 target `oatpp_meta_test`）
- `README.md` — 面向库使用者的项目文档（核心接口 / 用法示例 / 构建测试），改动对外接口时记得同步
- 根 `CMakeLists.txt` — 只设 C++11（`CMAKE_CXX_STANDARD 11`）并 `add_subdirectory(src test)`
- `test/CMakeLists.txt` 里 `META_BUILD_COMPILE_FAIL_PROBES` 选项及 `compile_fail/cf*.cpp`
  GLOB 是**历史残骸**：`test/compile_fail/` 目录已删除，该选项目前是空操作。

历史（git 中可查）：`expose_issues.cpp` / `test/compile_fail/`（cf01~cf06 探针）/
`benchmark/` / `src/oatpp_helper.hpp`、`traits.hpp`、`template_helper.hpp`、`md_operation.hpp`
均已随重构删除；测试已整体迁移到 `oatpp::meta::traits` 新接口（不再有"待迁移"或
`[expose]`/`[control]` 缺陷用例体系）。命名空间曾为 `oatpp::meta_operation`，现为 `oatpp::meta`；
代码注释中偶见的 CF01/CF03/CF06、ISSUE-* 编号只是历史背景。

## 构建与测试（已验证：configure / build / 22 用例 120 断言全部通过）

```bash
mkdir -p build && cd build          # 或用 CLion 的 cmake-build-debug/
cmake ..                            # 首次需联网拉取 oatpp 1.3.0 与 Catch2 v2.13.10
cmake --build .                     # 测试可执行文件在 build/test/oatpp_meta_test
./test/oatpp_meta_test              # 跑全部用例（默认全跑，无隐藏/必然失败用例）
./test/oatpp_meta_test "[null]"     # 按 tag 过滤；现有 tag：[traits] [null] [dto] [policy]
```

注意：oatpp / Catch2 由 FetchContent 下载到各构建目录的 `_deps/`（已存在则不再联网）。
根 `.gitignore` 只排除 `build/`，CLion 的 `cmake-build-debug/` 处于未跟踪状态。

## 编码约定与注意事项

- `src/` 是纯头文件库，CMake target **必须是 `INTERFACE`**（曾误用 `STATIC` 导致 ar 对空目标报错）。
- `src/CMakeLists.txt` 的 `target_sources` 只列出 `meta.hpp`：`meta_operation.hpp` 已重命名
  删除，若再引用它（或残留悬挂行）会直接导致 cmake configure 报 "Cannot find source file"。
- **代码风格：即使只有一行，`if` 语句也必须写大括号作用域**（禁止 `if (x) return;` 单行写法）。
- 头文件用 `#ifndef` include guard；`test/` 内以 `#include <meta.hpp>` 引用（include dir 已导出）。
- 构造风格：值初始化/聚合用 `T{}`；**泛型代码中单值构造调用用 `T(value)`**——oatpp 全部
  容器 wrapper 都有 `initializer_list` 构造函数，`{}` 会被 init-list 劫持静默改变语义。
- 平台：macOS / Apple Clang，库按 **C++11** 编写（`policy` 组合器用 tag dispatch 实现编译期
  分支，刻意不依赖 `if constexpr` / `std::is_same_v` 等 C++14/17 特性），编译错误信息以此为准。
