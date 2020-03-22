[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

# (Concepts-enabled) Functional Abstraction Layer for C++
Cefal is a C++20 header-only library with abstractions over basic functional programming concepts (and using C++20 concepts).

It is more a research pet project than a production-ready library. No tests are available yet.

### Dependencies
- **C++20**: Also requires concepts library as well
- **CMake** `>= 3.13.0`

## Available typeclasses

### Monoid
Has `empty` and `append` functions. For sake of performance `helpers::SingletonFrom` exists that can be used to wrap single element of monoidal container and pass it as right operand to append to avoid extra memory allocations.

#### Instances
 * `basic_types` - integral types and std::string
 * `std_containers` - single socket std:: containers
 * `std_optional` - std::optional
 * `with_functions` - any type that has `empty` and `append` methods

### Foldable
Has `foldLeft` function.

#### Instances
 * `std_containers` - single socket std:: containers
 * `with_functions` - any type that has `foldLeft` or `fold_left` method

### Functor
Has `unit` and `map` functions. Also provides `innerMap` function for Functor of Functors.

#### Instances
 * `from_foldable` - types that have instances for Monoid and Foldable
 * `std_optional` - std::optional
 * `with_functions` - any type that has `unit` and `map` methods

### Monad
Has `flatMap` function and also is a Functor. Also provides `innerFlatMap` function for Functor of Monads.

#### Instances
 * `from_foldable` - types that have instances for Monoid and Foldable
 * `std_optional` - std::optional
 * `with_functions` - any type that has `flatMap` or `flat_map` method

### Filterable
Has `filter` function. Also provides `innerFilter` function for Functor of Filterables.

#### Instances
 * `from_foldable` - types that have instances for Monoid and Foldable. Either SingletonFrom helper or Functor is also required.
 * `std_optional` - std::optional
 * `with_functions` - any type that has `filter` method

## Usage
All typeclasses are loaded from `cefal/cefal` header. No instances are loaded automatically, they need to be loaded on one-by-one basis.

All concepts are in `cefal::concepts` namespace.

All instances should be implemented in `cefal::instances` namespace.

All operations are in `cefal::ops` namespace and can be used either through pipe operator or with currying.

## Performance
No real benchmarks are created yet, but due to it being mostly a wrapper around std or user implementations - overhead is minimal.
For std::containers and map/filter operations few `non-pure` optimizations also are in place to provide performance similar to using pure std algorithms.

Ranges are not used due to their lack in any compiler at the current moment.

### Benchmarking
Toy "benchmarks" can be found in `src/dummy.cpp` which can't be used of course as proper measurement, but at least give an idea about performance. They are testing `std::vector<int>`, `std::set<int>` and `std::unordered_set<int>` performance in map/filter operations. All numbers are in milliseconds and is an average across 10 repeats (vectors are of 10kk elements and sets are of 100k elements).
 * `Transform` - just a `std::transform` to new container, no `reserve` calls
 * `Transform/reserve` - `std::transform` to new container with `reserve` on destination prior to transforming
 * `Transform/self` - `std::transform` to same container
 * `Map Immutable` - `ops::map` from lvalue (through `foldLeft`)
 * `Map Mutable` -  `ops::map` from rvalue (uses optimization bypassing the `foldLeft`)
 * `EraseIf Immutable` - container copying and `std::erase_if` on new container
 * `EraseIf Mutable` - `std::erase_if` on same container
 * `Filter Immutable` - `ops::filter` from lvalue (through `foldLeft`)
 * `Filter Mutable` - `ops::filter` from rvalue (uses optimization bypassing the `foldLeft`)

```
Vectors:
Transform         = 89
Map     Immutable = 51
Transform/reserve = 58
Map       Mutable = 21
Transform/self    = 20
Filter  Immutable = 34
EraseIf Immutable = 33
Filter    Mutable = 22
EraseIf   Mutable = 22

Sets:
Map     Immutable = 52
Transform         = 53
Map       Mutable = 44
Filter  Immutable = 39
EraseIf Immutable = 48
Filter    Mutable = 28
EraseIf   Mutable = 29

Unordered sets:
Map     Immutable = 47
Transform         = 48
Map       Mutable = 31
Filter  Immutable = 30
EraseIf Immutable = 34
Filter    Mutable = 18
EraseIf   Mutable = 19
```

### Observations on benchmark results
 * Mutable versions are on-par with std implementations
 * Immutable versions are either on-par or a bit faster (vector mapping and sets filtering) than std implementation
 * Mutable mapping exists for sets and can be 25% faster (for unordered_sets) than immutable one (std implementation doesn't have it due to limitations how `std::transform` works)

### Example
```cpp
std::vector<std::vector<int>> result =
        cefal::unit<std::vector>(3) | cefal::ops::map([](int x) { return ops::unit<std::vector>(x); })
                                    | cefal::ops::innerFilter([](int x){ return x % 2; })
                                    | cefal::ops::innerFlatMap([](int x){ return std::vector{x + 1, x + 2}; })
                                    | cefal::ops::innerMap([](int x){ return x * 3; });
```
