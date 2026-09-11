# oatpp-meta

[中文](../README.md) | **English**

A single-header **C++11** template metaprogramming library (CMake target: `oatpp_meta`, `INTERFACE`)
whose core capability is **type-level unwrapping / wrapping and runtime conversion** between std
types and oatpp wrapper types (`oatpp::Vector<T>`, `oatpp::String`, `oatpp::Int32`, `DTOWrapper`,
etc.); the recursion depth is selectable (deep by default, or outermost-only).

## Core interface: `oatpp::meta::traits<T, Recursion>`

Specializations are provided per oatpp type family, with the following members:

| Member | Description |
| --- | --- |
| `WrapperType` | the oatpp wrapper type itself |
| `UnwrapperType` | the std target type after unwrapping (`Recursion` decides whether nested containers are expanded) |
| `do_unwrapper(value, Policy)` | runtime unwrapping; the Policy decides scalar null semantics and is passed down the recursion |
| `do_wrapper(value)` | runtime reverse wrapping, packs `UnwrapperType` into `WrapperType`; always produces a non-null wrapper |

The second template parameter `Recursion` defaults to `recursion::deep`; pass `recursion::shallow`
for shallow mode (see "Recursion control").

The library provides specializations for the following oatpp types:

- Non-oatpp types: forwarded unchanged;
- `Primitive<T, Clazz>` such as `oatpp::Int8...Float64`: numeric primitives;
- `oatpp::String` / `oatpp::Boolean` / `EnumObjectWrapper`: scalar leaves;
- Container types (`oatpp::Vector/List/UnorderedSet`, `PairList`, `UnorderedMap`): elements and
  key/values are unwrapped recursively;
- `oatpp::Void` / `oatpp::Any`: cannot be statically unwrapped, kept as-is;
- DTO (`oatpp::Object<T>`): **user customization point** (see below).

Type-family flags derive from the common base `traits_base<type_category>`: all `is_xxx` constants
are derived automatically from the mutually exclusive `type_category` (passthrough / primitive /
scalar / container / object / opaque), so invalid combinations cannot be expressed.

### Usage example

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
oatpp::Vector<oatpp::Int32> w = traits<oatpp::Vector<oatpp::Int32>>::do_wrapper(back); // non-null wrapper
```

## Recursion control: deep vs shallow

An oatpp container's elements may themselves be containers or DTOs, and not all of them have an
unwrapping target; to control the recursion depth, the second template parameter of
`traits<T, Recursion>` offers two modes, `deep` / `shallow`.

- `recursion::deep` (default): elements and key/values are unwrapped recursively;
- `recursion::shallow`: only the outermost container is converted; elements keep their oatpp
  wrapper types.

```cpp
using oatpp::meta::shallow_traits;   // == traits<T, recursion::shallow>

// deep (default)
traits<oatpp::Vector<oatpp::Vector<oatpp::Int32>>>::UnwrapperType
    // std::vector<std::vector<std::int32_t>>

// shallow
shallow_traits<oatpp::Vector<oatpp::Vector<oatpp::Int32>>>::UnwrapperType
    // std::vector<oatpp::Vector<oatpp::Int32>>
```

A container of DTOs needs no `traits` specialization for that DTO:

```cpp
// MyDto has no traits specialization; deep would trigger the DTO customization static_assert
std::vector<oatpp::Object<MyDto>> v =
    shallow_traits<oatpp::Vector<oatpp::Object<MyDto>>>::do_unwrapper(dtos);
```

Shallow semantics:

- elements (including null ones) are kept as-is and **never enter the Policy**, so null elements
  do not throw;
- a null container still becomes an empty container;
- a null inner container **stays null** and is not turned into an empty container;
- leaf types (primitive / scalar / opaque) have no recursion, so shallow is equivalent to deep.

## Null handling

An oatpp object is essentially a pointer carrying runtime type information, so every oatpp object
can be null. To let callers customize what happens to a null object, oatpp-meta provides policies
per type family:

1. Containers are converted to empty containers;
2. Null scalars are handled by a **Policy**:
   - default `null_to_throw`: treated as a data contract violation, throws `null_unwrap_error`;
   - leniency must be requested explicitly via `null_to_default{}`: mapped to a default value
     (loses the distinction between null and zero);
   - a custom policy only has to provide `template<typename U> static U on_null_scalar()`;
3. Null DTO values are handled by the user's custom conversion code;

```cpp
oatpp::Int32 n;                                        // null scalar
traits<oatpp::Int32>::do_unwrapper(n);                 // throws null_unwrap_error
traits<oatpp::Int32>::do_unwrapper(n, oatpp::meta::null_to_default{}); // 0
```

### Per-type dispatch: `policy::combine`

Specify different policies for different scalar types within a single unwrap call. The first
matching entry in declaration order wins:

```cpp
using my_policy = oatpp::meta::policy::combine<
    oatpp::meta::policy::for_type<std::int32_t, oatpp::meta::null_to_default>,
    oatpp::meta::policy::for_type<std::string, oatpp::meta::null_to_default>,
    oatpp::meta::policy::otherwise<oatpp::meta::null_to_throw>
>;

auto result = traits<SomeOatppType>::do_unwrapper(value, my_policy{});
```

## DTO customization point

DTO fields have no uniform C++ target type and can only be specified by the user; to convert DTO
objects uniformly, oatpp-meta exposes a customization point: inherit the helper base
`dto_traits_base<MyDto, MyStruct>`, specialize `oatpp::meta::traits`, and implement `do_unwrapper`
(plus optional `do_wrapper`). Example:

```cpp
namespace oatpp { namespace meta {
template<>
struct traits<oatpp::Object<TestDto>> : dto_traits_base<TestDto, TestStruct> {
    template<typename Policy = null_to_throw>
    static UnwrapperType do_unwrapper(oatpp::Object<TestDto> const &value, Policy const & = {}) {
        TestStruct result{};
        if (value) {
            result.id = traits<oatpp::Int32>::do_unwrapper(value->id);
            // ... unwrap field by field (null semantics are up to this specialization)
        }
        return result;
    }
};
}}
```

If you only want a `std::vector<oatpp::Object<MyDto>>` without field-level conversion, use
`shallow_traits<oatpp::Vector<oatpp::Object<MyDto>>>` directly and skip the specialization above
(see "Recursion control").

## oatpp dependency

Following the official oatpp components (such as oatpp-swagger), the library lets you choose where
the oatpp dependency comes from at configure time via the `OATPP_MODULES_LOCATION` variable:

- `AUTO` (default): probes automatically, in order. An installed oatpp 1.3.0 or newer is used if
  present; otherwise it falls back to `EXTERNAL`.
- `INSTALLED`: uses the installed oatpp found by `find_package` (point `oatpp_DIR` at a custom
  install location if needed).
- `EXTERNAL`: downloads and builds oatpp from GitHub during the build. It fetches `origin/master`
  by default; pin a version or branch with `OATPP_GIT_TAG`. Note: oatpp's master has moved
  `oatpp/core/Types.hpp` to `oatpp/Types.hpp`, so the default master currently does not compile
  this library; pin `-DOATPP_GIT_TAG=1.3.0` for an actual build.
- `CUSTOM`: uses local oatpp headers and library. `OATPP_DIR_SRC` is the directory containing
  oatpp headers (such as `<oatpp sources>/src`) and `OATPP_DIR_LIB` is the directory containing
  `liboatpp`.

Common configure commands:

```bash
cmake ..                                                          # default AUTO
cmake -DOATPP_MODULES_LOCATION=EXTERNAL ..                        # explicitly download and build
cmake -DOATPP_MODULES_LOCATION=EXTERNAL -DOATPP_GIT_TAG=1.3.0 ..  # pin the remote version
cmake -DOATPP_MODULES_LOCATION=CUSTOM \
      -DOATPP_DIR_SRC=<oatpp sources>/src -DOATPP_DIR_LIB=<oatpp lib dir> ..  # local sources/library
```

An invalid value, a missing installed oatpp in `INSTALLED` mode, or a missing or incorrect
directory in `CUSTOM` mode all produce a guiding error at configure time. When the project is
consumed as a subdirectory (`add_subdirectory`) and the caller already provides an oatpp target,
that target is reused and nothing is searched or downloaded.

## Build and test

```bash
mkdir -p build && cd build          # or CLion's cmake-build-debug/
cmake ..                            # default AUTO: with no installed oatpp, the build stage downloads and compiles one
cmake --build .                     # test binary: build/test/oatpp_meta_test
./test/oatpp_meta_test              # all tests (currently 28 cases / 167 assertions passing)
./test/oatpp_meta_test "[null]"     # filter by tag; current tags: [traits] [null] [dto] [policy] [shallow]
```

- Language standard: C++11 (`CMAKE_CXX_STANDARD 11`); oatpp and Catch2 are also compiled as C++11.
- To use the library, only `src/meta.hpp` is needed; the library itself depends only on oatpp,
  see above for choosing its source.
- Development environment: macOS / Apple Clang; CI (GitHub Actions) builds and tests on ubuntu-latest and macOS-latest.

## License

[Apache License 2.0](../LICENSE)
