# compile_fail —— 预期编译失败的缺陷探针

本目录用于存放**预期编译失败**的探针文件。每个 `.cpp` 文件对应一个已实证的编译期缺陷或设计行为，它们在修复/落地前**无法通过编译**，这正是目的所在。

> 当前本目录下无活跃探针（cf01~cf06 已随旧接口重构全部移除，CF04 的 static_assert 诊断已转为 `test.cpp` 中 DTO 定制点的运行时测试验证）。

## 使用方式

默认不参与构建。启用后每个探针是独立的 target（且 `EXCLUDE_FROM_ALL`，不会污染 "Build All"）：

```bash
cd cmake-build-debug
cmake -DMETA_BUILD_COMPILE_FAIL_PROBES=ON ..
cmake --build . --target <probe_name>   # 单独构建某个探针，观察编译错误
```

## 修复后的"毕业"流程

1. 修复某个缺陷；
2. 对应探针编译通过 → 把探针中的场景改写为 Catch2 运行时测试（移入 `../test.cpp`）；
3. 从本目录删除对应探针文件。
