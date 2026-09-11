# Contributing

[中文](../CONTRIBUTING.md) | **English**

Thanks for your interest in oatpp-meta. This document explains how to report issues, submit
code, and verify changes locally.

## Reporting issues

File an issue via [GitHub Issues](https://github.com/SoTTT/meta/issues) using the matching
template. Please include:

- Compiler and version (e.g. Apple Clang 15 / GCC 13 / MSVC 19.4x) and operating system
- oatpp version and dependency source (your `OATPP_MODULES_LOCATION` value)
- A minimal reproducible snippet or test case

## Submitting code

1. Fork the repository and branch off `master`.
2. Commit messages are written in Chinese, following Conventional Commits
   (e.g. `feat(traits): ...`, `fix(cmake): ...`).
3. Code comments are written in Chinese; `if` statements always use braces, even for a
   single statement.
4. The library targets C++11 — do not use C++14/17 features (`if constexpr`,
   `std::is_same_v`, etc.).
5. In generic code, construct single values with `T(value)` rather than `T{value}` (oatpp
   container wrappers have initializer_list constructors that hijack `{}`).
6. New externally visible behavior requires test cases and synchronized updates to both
   `README.md` and `i18n/README.en.md`.

## Local verification

```bash
mkdir -p build && cd build
cmake -DOATPP_MODULES_LOCATION=EXTERNAL -DOATPP_GIT_TAG=1.3.0 ..
cmake --build . -j
./test/oatpp_meta_test          # all tests must pass
```

Note: EXTERNAL mode fetches oatpp master by default, which is currently incompatible with
this library (header paths have moved). Always pin `-DOATPP_GIT_TAG=1.3.0`.

## Before merging

Pull requests are built and tested by GitHub Actions on ubuntu-latest, macos-latest, and
windows-latest. Please keep CI green.
