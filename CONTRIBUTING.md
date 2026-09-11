# 贡献指南

**中文** | [English](i18n/CONTRIBUTING.en.md)

感谢你对 oatpp-meta 的兴趣。本文说明如何报告问题、提交代码与在本地验证改动。

## 报告问题

通过 [GitHub Issues](https://github.com/SoTTT/meta/issues) 提交，请使用对应模板并尽量提供：

- 编译器与版本（如 Apple Clang 15 / GCC 13 / MSVC 19.4x）、操作系统
- oatpp 版本与依赖来源（`OATPP_MODULES_LOCATION` 取值）
- 最小可复现的代码或测试用例

## 提交代码

1. Fork 仓库并创建分支，基于 `master` 开发。
2. 提交信息用中文，遵循 Conventional Commits（如 `feat(traits): ...`、`fix(cmake): ...`）。
3. 代码注释用中文；`if` 语句即使只有一行也必须写大括号作用域。
4. 库按 C++11 编写，不要使用 C++14/17 特性（如 `if constexpr`、`std::is_same_v`）。
5. 泛型代码中单值构造调用用 `T(value)` 而非 `T{value}`（oatpp 容器 wrapper 的
   initializer_list 构造函数会劫持 `{}`）。
6. 新增对外行为需补充测试用例，并同步更新 `README.md` 与 `i18n/README.en.md`。

## 本地验证

```bash
mkdir -p build && cd build
cmake -DOATPP_MODULES_LOCATION=EXTERNAL -DOATPP_GIT_TAG=1.3.0 ..
cmake --build . -j
./test/oatpp_meta_test          # 全部用例须通过
```

注意：EXTERNAL 模式默认拉取 oatpp master，目前与本库不兼容（头文件路径已迁移），
必须固定 `-DOATPP_GIT_TAG=1.3.0`。

## 合并前检查

PR 合并前会通过 GitHub Actions 在 ubuntu-latest、macos-latest、windows-latest
三个平台构建并运行全部测试，请确保 CI 全绿。
