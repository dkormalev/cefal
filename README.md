[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

# (Concepts-enabled) Functional Abstraction Layer for C++
Cefal is a C++20 header-only library with abstractions over basic functional programming concepts (and using C++20 concepts).

It is more a research pet project than a production-ready library (especially keeping in mind it compiles only on GCC/master for now).

Tests exist though and [benchmarks](#performance) as well.

See [examples](#examples) for general idea about what it looks like or check `src/dummy.cpp`.

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
 * `std_ranges` - std::ranges::views
 * `with_functions` - any type that has `foldLeft` or `fold_left` method

### Functor
Has `unit` and `map` functions. Also provides `innerMap` function for Functor of Functors.

#### Instances
 * `from_foldable` - types that have instances for Monoid and Foldable
 * `std_optional` - std::optional
* `std_ranges` - std::ranges::views
  * `with_functions` - any type that has `unit` and `map` methods

### Monad
Has `flatMap` function and also is a Functor. Also provides `innerFlatMap` function for Functor of Monads.

#### Instances
 * `from_foldable` - types that have instances for Monoid and Foldable
 * `std_optional` - std::optional
 * `with_functions` - any type that has `flatMap` or `flat_map` method and also is a Functor

### Filterable
Has `filter` function. Also provides `innerFilter` function for Functor of Filterables.

#### Instances
 * `from_foldable` - types that have instances for Monoid and Foldable. Either SingletonFrom helper or Functor is also required.
 * `std_optional` - std::optional
 * `std_ranges` - std::ranges::views
 * `with_functions` - any type that has `filter` method

## Usage
All typeclasses can be loaded with `cefal/cefal` header. No instances are loaded automatically, they need to be loaded on one-by-one basis (`cefal/everything.h` exists though with all the instances added, but is not recommended to use).

All concepts are in `cefal::concepts` namespace.

All instances should be implemented in `cefal::instances` namespace.

All operations are in `cefal::ops` namespace and can be used either through pipe operator or with currying.

### Lvalue vs rvalue
All operations on lvalue operands expect constref arguments of functions, passed to them (except accumulator for foldLeft, which is rvalue).

All operations on rvalue operands can work with rvalue as well.

The only exception is operations on ranges. They are done in compliance with how ranges work and on both lvalue and rvalue expect either constref or ref (from where it is possible to move).
Be aware though that moving from ranges operation sometimes can be more expensive than copying due to extensive optimizations compilers could do on ranges. For example, check mutable vs immutable benchmarks for `map()` on ranges for case when it works with `Expensive<int>` and converts it to different type. On `-O3` level immutable benchmark is faster roughly 2-3 times.

## Performance
Due to cefal being mostly a wrapper around std or user implementations - overhead should be minimal.

For std::containers and map/filter operations few non-pure optimizations are in place to provide performance similar to using `std` algorithms. Cefal also contains Catch2-based benchmarks for std::containers as for something that can be both heavy enough to process and comparable with other implementation (`std` algorithms).

Ranges-based benchmarks are available as well, but their numbers are not presented below due to being the same across `std::ranges::views` and `cefal::ops`.

For benchmarks we use next value types:
 * int - as an example of lightweight type without any extra memory allocations
 * Expensive - custom type that has memory allocation performed in constructor and copy constructor, but can be cheaply moved
 * For Maps of expensive type we use int as key and Expensive as value. There is one exception - extra map2 in `map()` for Expensive as key and int as value.

Container sizes are not the same for different containers (otherwise it would either take too much time for slow ones or too less for fast ones), so different containers can't be compared, but containers used for cefal and std are the same size:
 * std::vector: 10kk for int and 25k for Expensive
 * std::list: 1kk for int and 25k for Expensive
 * std::deque: 10kk for int and 25k for Expensive
 * std::set: 100k for int and 25k for Expensive
 * std::unordered_set: 100k for int and 25k for Expensive
 * std::map: 100k for int and 25k for Expensive
 * std::unordered_map: 100k for int and 25k for Expensive


Multi versions of sets are also benchmarked, but are similar to single-entry sets and are omitted in results below for brevity.

There are two types of benchmarks:
 * Immutable - initial container is taken by lvalue and is not modified
 * Mutable - Initial container is taken by rvalue and can be modified

Std references:
 * For `map()` we use `std::transform` (either to new or to same container). If it is vector and immutable - we `reserve()` destination as well
 * For `filter()` we use `std::erase_if` either on copy of container or on initial container

Map benchmarks transform container to same type (to make it similar between lvalue and rvalue).

Filter benchmarks are also divided by percentage of items that are accepted.

All values are from `mean` section of catch2 benchmarks.

Std library is from GCC/master (commit 73dd051894b8293d35ea1c436fa408c404b80813, April 1 2020) and all benchmarks are built with `-O3`.

### map() for int
Type            | vector   | list     | deque    | set      | unordered_set  | map      | unordered_map
--------------- | -------: | -------: | -------: | -------: | -------------: | -------: | --------------:
Immutable cefal | 23.028   | 183.425  | 58.212   | 27.229   | 19.808         | 27.218   | 19.742
Immutable std   | 27.566   | 182.485  | 57.714   | 26.004   | 17.750         | 26.439   | 17.324
Mutable cefal   | 2.998    | 4.191    | 6.875    | 27.561   | 20.442         | 28.143   | 20.528
Mutable std     | 3.007    | 4.258    | 9.540    | N/A      | N/A            | N/A      | N/A

### map() for Expensive
Type            | vector    | list      | deque     | set       | unordered_set  | map      | map 2    | unordered_map
--------------- | --------: | --------: | --------: | --------: | -------------: | -------: | -------: | --------------:
Immutable cefal | 60.766    | 65.032    | 61.077    | 67.672    | 66.641         | 72.256   | 70.414   | 66.610
Immutable std   | 67.383    | 65.151    | 61.076    | 69.518    | 66.037         | 71.156   | 81.959   | 66.226
Mutable cefal   | 0.052     | 0.102     | 0.034     | 5.185     | 5.054          | 6.800    | 5.869    | 5.204
Mutable std     | 15.692    | 16.074    | 16.077    | N/A       | N/A            | N/A      | N/A      | N/A

### filter() for int
Type              | 10%      | 25%      | 50%      | 75%      | 90%
----------------- | -------: | -------: | -------: | -------: | ---------:
**vector**        |||||
Immutable cefal   | 40.744   | 43.751   | 49.754   | 53.399   | 55.552
Immutable std     | 45.551   | 45.623   | 48.697   | 48.261   | 48.471
Mutable cefal     | 37.466   | 37.475   | 37.419   | 37.514   | 37.774
Mutable std       | 37.247   | 37.446   | 40.523   | 40.479   | 40.574
**list**          |||||
Immutable cefal   | 21.108   | 47.129   | 94.010   | 139.376  | 164.344
Immutable std     | 185.323  | 191.391  | 196.062  | 191.282  | 184.193
Mutable cefal     | 98.675   | 83.869   | 59.218   | 30.941   | 14.719
Mutable std       | 88.463   | 79.781   | 61.182   | 24.538   | 10.800
**deque**         |||||
Immutable cefal   | 47.505   | 53.606   | 64.657   | 76.670   | 79.651
Immutable std     | 88.113   | 86.789   | 86.097   | 86.738   | 83.507
Mutable cefal     | 59.539   | 56.142   | 51.801   | 46.972   | 43.073
Mutable std       | 59.425   | 56.518   | 51.882   | 47.131   | 43.188
**set**           |||||
Immutable cefal   | 4.142    | 7.353    | 13.856   | 20.538   | 25.234
Immutable std     | 24.654   | 23.563   | 22.448   | 21.922   | 20.884
Mutable cefal     | 11.688   | 9.444    | 6.159    | 3.257    | 2.109
Mutable std       | 11.397   | 9.375    | 6.332    | 3.249    | 2.075
**unordered_set** |||||
Immutable cefal   | 4.497    | 6.823    | 10.945   | 14.873   | 19.134
Immutable std     | 17.767   | 17.242   | 16.952   | 17.331   | 16.105
Mutable cefal     | 9.191    | 7.583    | 4.294    | 2.411    | 1.219
Mutable std       | 9.211    | 7.589    | 4.337    | 2.531    | 1.173
**map** |||||
Immutable cefal   | 4.380    | 8.136    | 14.588   | 21.912   | 26.258
Immutable std     | 24.813   | 23.844   | 22.642   | 22.029   | 21.045
Mutable cefal     | 11.642   | 9.634    | 6.436    | 3.591    | 2.157
Mutable std       | 11.673   | 9.476    | 6.278    | 3.345    | 2.078
**unordered_map** |||||
Immutable cefal   | 4.831    | 7.403    | 11.795   | 16.069   | 19.637
Immutable std     | 18.383   | 18.710   | 17.370   | 16.854   | 16.475
Mutable cefal     | 9.716    | 7.978    | 4.654    | 2.555    | 1.308
Mutable std       | 9.607    | 8.018    | 4.551    | 2.499    | 1.230

### filter() for Expensive
Type              | 10%      | 25%      | 50%      | 75%      | 90%
----------------- | -------: | -------: | -------: | -------: | ---------:
**vector**        |||||
Immutable cefal   | 11.629   | 29.254   | 58.915   | 89.071   | 106.378
Immutable std     | 68.323   | 114.938  | 283.826  | 97.746   | 64.673
Mutable cefal     | 34.500   | 56.911   | 157.366  | 38.797   | 8.523
Mutable std       | 34.479   | 56.636   | 156.910  | 38.953   | 8.597
**list**          |||||
Immutable cefal   | 6.208    | 15.706   | 31.769   | 47.632   | 57.195
Immutable std     | 65.371   | 66.365   | 67.938   | 66.414   | 65.897
Mutable cefal     | 40.173   | 81.944   | 241.911  | 46.628   | 9.073
Mutable std       | 32.853   | 28.317   | 20.106   | 10.978   | 5.090
**deque**         |||||
Immutable cefal   | 6.014    | 15.288   | 29.776   | 45.084   | 53.986
Immutable std     | 69.170   | 118.254  | 286.198  | 100.668  | 65.932
Mutable cefal     | 39.759   | 80.341   | 240.852  | 47.481   | 8.812
Mutable std       | 39.699   | 78.960   | 244.601  | 47.200   | 8.718
**set**           |||||
Immutable cefal   | 21.834   | 31.497   | 48.266   | 65.367   | 76.619
Immutable std     | 68.914   | 68.631   | 67.969   | 67.644   | 67.901
Mutable cefal     | 33.821   | 29.160   | 20.652   | 11.234   | 5.362
Mutable std       | 33.879   | 29.125   | 20.672   | 11.202   | 5.342
**unordered_set** |||||
Immutable cefal   | 21.802   | 31.021   | 47.717   | 78.145   | 77.942
Immutable std     | 68.047   | 67.801   | 68.694   | 69.279   | 67.614
Mutable cefal     | 33.652   | 28.704   | 20.570   | 11.367   | 5.485
Mutable std       | 33.440   | 28.778   | 21.765   | 11.225   | 5.457
**map** |||||
Immutable cefal   | 7.176    | 18.088   | 34.954   | 53.145   | 63.102
Immutable std     | 83.866   | 87.060   | 82.871   | 83.906   | 85.545
Mutable cefal     | 33.768   | 30.118   | 21.740   | 11.470   | 5.764
Mutable std       | 48.071   | 43.940   | 35.722   | 25.031   | 19.477
**unordered_map ** |||||
Immutable cefal   | 7.502    | 17.510   | 34.283   | 63.619   | 65.634
Immutable std     | 84.983   | 85.647   | 85.597   | 84.457   | 83.882
Mutable cefal     | 35.043   | 30.765   | 21.955   | 11.599   | 5.544
Mutable std       | 50.346   | 44.926   | 36.233   | 25.999   | 19.650

### Observations on benchmark results
 * Immutable `map()` is on par with `std::transform`
 * Mutable `map()` for ints is on par, but for move-efficient types it is A LOT faster than `std::transform` (numbers in table above are not a mistake)
 * There is no mutable `std::transform` for set-like containers, but mutable `map()` for ints is roughly the same as immutable versions of both cefal and std
 * Mutable `map()` for set-like containers of move-efficient types is much faster than immutable cefal or std performance
 * For maps - performance is similar as for set-like containers. There is one extra interesting point - `std::transform` for `std::map` of Expensive as key works slower than for case when Expensive is value. It doesn't happen for `cefal::map()` and not reproducible on `std::unordered_map` or on any filter/erase_if operations.
 * `filter()` is worse than `std::erase_if` for mutable lists of both ints and Expensive and for vectors of Expensive in case when almost whole container is accepted
 * `filter()` is on par with `std::erase_if` in case of immutable vector of small types and in case of all other mutable containers not mentioned above
 * For immutable containers except vector `filter()` is either on par or better than `std::erase_if`. Less elements are accepted - bigger the gap for in favor of `filter()` (up to 10x in case of 10% elements accepted)
 * Benchmarks for mapping to another inner type also exist in source codes (not added here for brevity). For immutable containers they show pretty much the same results (i.e. almost equal between cefal and std). Mutable cefal benchmarks for move-effective types though shows 1.5x-2x better performance on all containers except unordered_set.

As a general conclusion: there are for sure few cases where cefal shows itself worse than direct usage of std algorithm (not tremendously though), but there are also a lot of cases where cefal works faster by 1-2 orders of magnitude (especially in case of move-efficient types) and in remaining cases it is on par with std.

Cefal lacks laziness, but it can be achieved with `std::ranges` (cefal has partial support for them as `Foldable`, `Functor` and `Filterable`).

## Examples

### Piped form
```cpp
std::vector<std::vector<int>> result =
    cefal::unit<std::vector>(3) | cefal::ops::map          ([](int x) { return ops::unit<std::vector>(x); })
                                | cefal::ops::innerFilter  ([](int x) { return x % 2; })
                                | cefal::ops::innerFlatMap ([](int x) { return std::vector{x + 1, x + 2}; })
                                | cefal::ops::innerMap     ([](int x) { return x * 3; });
```
```cpp
auto rawToResult = cefal::ops::flatMap([](RawResult&& raw){ return maybeGetResult(std::move(raw)); };
std::optional<int> result = maybeGetRawResult() | rawToResult
                                                | cefal::ops::map([](Result&& x) { return x.value(); });
```

### Curried form
```cpp
auto mapper = cefal::ops::map([](int x) { return x * 3; });
auto result = mapper(cefal::unit<std::vector>(3));
```
```cpp
auto left = cefal::unit<std::vector>(3);
auto anotherResult = cefal::ops::map([](int x) { return x * 3; })(left);
```

### Custom class support
```cpp
template <typename T>
class MyClass {
  // ...
};

namespace cefal::instances {
template <typename T>
struct Functor<MyClass<T>> {
  static MyClass<T> unit(T x) {
    MyClass<T> result;
    result.setValue(std::move(x));
    return result;
  }

  static auto map(const MyClass<T>& src, Func&& func) {
    using U = std::invoke_result_T<Func, T>;
    MyClass<U> result;
    result.setValue(func(src.value()));
    return result;
  }
};
}

MyClass<int> from = cefal::ops::unit<MyClass>(42);
MyClass<double> result = from | cefal::ops::map([](int x) -> double { return x * 2.0; });
```

### Custom class support through class methods
```cpp
template <typename T>
class MyClass {
  static MyClass<T> unit(T x) {
    MyClass<T> result;
    result.setValue(std::move(x));
    return result;
  }

  auto map(Func&& func) {
    using U = std::invoke_result_T<Func, T>;
    MyClass<U> result;
    result.setValue(func(value()));
    return result;
  }
  // ...
};

MyClass<int> from = cefal::ops::unit<MyClass>(42);
MyClass<double> result = from | cefal::ops::map([](int x) -> double { return x * 2.0; });
```
