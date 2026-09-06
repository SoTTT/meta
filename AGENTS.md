# AGENTS.md — meta

## 项目概述

`meta` 是一个**纯头文件 C++17 模板元编程库**（target 名 `meta_operation`），核心功能是在
std 类型与 oatpp 包装类型（`oatpp::Vector<T>`、`oatpp::String`、`DTOWrapper` 等）之间做
**类型层深解包/深包装与运行时转换**。仅依赖 oatpp 1.3.0（通过 CMake FetchContent 拉取）。
代码注释与提交信息为中文，保持一致。

核心接口：`oatpp::meta::traits<T>`（`src/meta_operation.hpp`）——按 oatpp 类型族
提供模板特化，成员包括 `WrapperType` / `UnwrapperType` / `do_unwrapper` / `do_wrapper` /
`type_category` 分类枚举（全部 `is_xxx` 标志由 `traits_base<C>` 基类派生）。
DTO 的解包是**用户定制点**：对自己的 DTO 全特化 `traits`（继承 `dto_traits_base`）。
null 语义为三层模型：容器 null → 空容器（内建）；标量 null → Policy 决定
（默认 `null_to_throw`，宽容需显式传 `null_to_default{}`）；DTO 字段语义归用户。

## 目录结构

- `src/` — 库本体：单一头文件 `oatpp_helper.hpp`（无 .cpp）
- `test/` — Catch2 v2 测试：
  - `test.cpp` — 常规测试（含 `CATCH_CONFIG_MAIN`）
  - `expose_issues.cpp` — **已知缺陷的运行时用例**（见下方"缺陷管理约定"）
  - `compile_fail/` — **预期编译失败的探针**，每个文件对应一个已实证缺陷
- ⚠️ 测试套件（test.cpp / expose_issues.cpp / cf01~cf06 探针）大多仍基于已重构掉的旧接口
  （`meta_operation::type_traits::oatpp::unwrapper` 等），**当前无法编译，待迁移到
  `oatpp::meta::traits` 新接口**；cf04_dto.cpp 已迁移，可作为迁移样板。

历史：`traits.hpp` / `template_helper.hpp` / `md_operation.hpp` / `benchmark/` 已于
重构中移除（多维容器操作产品线砍除，Abseil 依赖随之解除），git 历史中可查。

## 构建与测试

```bash
mkdir -p build && cd build          # 或用 CLion 的 cmake-build-debug/
cmake ..
cmake --build .
./test/meta_operation_test                        # 运行常规测试（迁移完成后）
./test/meta_operation_test "[expose]"             # 只跑必然失败的缺陷用例
./test/meta_operation_test "~[.]"                 # 排除隐藏的崩溃用例（默认行为）
```

单独构建编译失败探针（观察某个缺陷的编译错误）：

```bash
cmake -DMETA_BUILD_COMPILE_FAIL_PROBES=ON ..
cmake --build . --target cf04_dto    # 当前仅 cf04 与新接口同步
```

注意：`META_BUILD_COMPILE_FAIL_PROBES` 默认 **OFF**，开启后探针也是 `EXCLUDE_FROM_ALL`，
不会污染 Build All。

## 缺陷管理约定（改动测试前必读）

本仓库把"已知未修复的缺陷"作为一等公民管理，不要随意"修复"测试让它们变绿：

1. **`test/expose_issues.cpp` 的三类 tag**：
   - `[expose]` — 当前必然失败的断言，对应已确认缺陷；修复缺陷后才应转绿并移入常规测试
   - `[.][crash]` — Catch2 隐藏用例，运行会 **SIGSEGV 杀死整个测试进程**，因此默认不跑；
     修复 null 语义后才去掉 `[.]`
   - `[control]` — 对照组，当前必须通过，修复缺陷时不得破坏
2. **`test/compile_fail/` 探针毕业流程**（详见该目录 README.md）：修复缺陷 → 探针编译通过 →
   把场景改写为 Catch2 运行时测试（移入 `test.cpp` 或 `expose_issues.cpp` 的 `[control]`）→
   删除探针文件。例外：CF04 按 ISSUE-DTO 修复方向 (a) 落地后**永久保留**——DTO 默认
   static_assert 是设计行为，探针用于锁定诊断信息质量。
3. 缺陷清单编号：CF01~CF06（编译期）+ ISSUE-NULL-NUMERIC / ISSUE-NULL-CONTAINER /
   ISSUE-NULL-STRING / ISSUE-DTO（运行时/类型层），对应关系见 `expose_issues.cpp` 头部注释。
   新接口下 CF01/02/03/05/06 与 ISSUE-NULL-* 的能力缺口均已补齐，待测试迁移时走毕业流程。

## 编码约定与注意事项

- `src/` 是纯头文件库，CMake target **必须是 `INTERFACE`**（曾误用 `STATIC` 导致 ar 对空目标报错）。
- **代码风格：即使只有一行，`if` 语句也必须写大括号作用域**（禁止 `if (x) return;` 这种单行写法）。
- 头文件用 `#ifndef` include guard；`src/` 内互相引用可用 `<oatpp_helper.hpp>` 形式（include dir 已导出）。
- 构造风格约定：值初始化/聚合用 `T{}`；**泛型代码中单值构造调用用 `T(value)`**——oatpp 全部
  容器 wrapper 都有 `initializer_list` 构造函数，`{}` 会被 init-list 劫持静默改变语义。
- 平台：开发环境为 macOS / Apple Clang / C++17，缺陷清单中的编译错误信息以此为准。
