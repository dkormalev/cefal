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

All operations are in `cefal::ops` namespace.

### Example
```cpp
std::vector<std::vector<int>> result =
        cefal::unit<std::vector>(3) | cefal::ops::map([](int x) { return ops::unit<std::vector>(x); })
                                    | cefal::ops::innerFilter([](int x){ return x % 2; })
                                    | cefal::ops::innerFlatMap([](int x){ return std::vector{x + 1, x + 2}; })
                                    | cefal::ops::innerMap([](int x){ return x * 3; });
```
