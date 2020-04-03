[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

# (Concepts-enabled) Functional Abstraction Layer for C++
Cefal is a C++20 header-only library with abstractions over basic functional programming concepts (and using C++20 concepts).

It is more a research pet project than a production-ready library (especially keeping in mind it compiles only on GCC/master for now).

Tests exist though and [benchmarks](#performance) as well.

Ranges are not used and/or supported due to their lack in any compiler at the current moment, but support for them is going to be seriously considered.

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
 * `with_functions` - any type that has `flatMap` or `flat_map` method and also is a Functor

### Filterable
Has `filter` function. Also provides `innerFilter` function for Functor of Filterables.

#### Instances
 * `from_foldable` - types that have instances for Monoid and Foldable. Either SingletonFrom helper or Functor is also required.
 * `std_optional` - std::optional
 * `with_functions` - any type that has `filter` method

## Usage
All typeclasses can be loaded with `cefal/cefal` header. No instances are loaded automatically, they need to be loaded on one-by-one basis (`cefal/everything.h` exists though with all the instances added, but is not recommended to use).

All concepts are in `cefal::concepts` namespace.

All instances should be implemented in `cefal::instances` namespace.

All operations are in `cefal::ops` namespace and can be used either through pipe operator or with currying.

## Performance
Due to cefal being mostly a wrapper around std or user implementations - overhead should be minimal.

For std::containers and map/filter operations few non-pure optimizations are in place to provide performance similar to using `std` algorithms. Cefal also contains Catch2-based benchmarks for std::containers as for something that can be both heavy enough to process and comparable with other implementation (`std` algorithms).

For benchmarks we use next value types:
 * int - as an example of lightweight type without any extra memory allocations
 * Expensive - custom type that has memory allocation performed in constructor and copy constructor, but can be cheaply moved

Container sizes are not the same for different containers (otherwise it would either take too much time for slow ones or too less for fast ones), so different containers can't be compared, but containers used for cefal and std are the same size:
 * std::vector: 10kk for int and 25k for Expensive
 * std::list: 1kk for int and 25k for Expensive
 * std::deque: 10kk for int and 25k for Expensive
 * std::set: 100k for int and 25k for Expensive
 * std::unordered_set: 100k for int and 25k for Expensive

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

Std library is from GCC/master (commit 5c7e6d4bdf879b437b43037e10453275acabf521, March 12 2020) and all benchmarks are built with `-O3`.

### map() for int
Type            | vector   | list     | deque    | set      | unordered_set
--------------- | -------: | -------: | -------: | -------: | --------------:
Immutable cefal | 21.718   | 171.495  | 57.854   | 26.636   | 19.232
Immutable std   | 21.864   | 173.466  | 56.057   | 25.520   | 16.818
Mutable cefal   | 2.737    | 3.507    | 6.088    | 22.714   | 16.854
Mutable std     | 2.726    | 3.563    | 8.651    | N/A      | N/A

### map() for Expensive
Type            | vector    | list      | deque     | set       | unordered_set
--------------- | --------: | --------: | --------: | --------: | --------------:
Immutable cefal | 57.689    | 88.777    | 85.942    | 95.224    | 94.896
Immutable std   | 58.249    | 88.574    | 88.007    | 95.759    | 94.695
Mutable cefal   | 0.028     | 0.100     | 0.029     | 4.909     | 5.199
Mutable std     | 40.646    | 48.948    | 56.955    | N/A       | N/A

### filter() for int
Type              | 10%      | 25%      | 50%      | 75%      | 90%
----------------- | -------: | -------: | -------: | -------: | ---------:
**vector**        |||||
Immutable cefal   | 40.147   | 43.507   | 48.945   | 52.789   | 54.718
Immutable std     | 48.194   | 47.940   | 48.160   | 47.780   | 47.890
Mutable cefal     | 37.207   | 38.674   | 37.489   | 37.347   | 37.347
Mutable std       | 40.250   | 40.472   | 40.489   | 40.470   | 40.662
**list**          |||||
Immutable cefal   | 90.874   | 232.790  | 479.898  | 744.863  | 908.179
Immutable std     | 938.055  | 978.817  | 1004.820 | 989.505  | 1000.19
Mutable cefal     | 208.301  | 183.658  | 108.289  | 49.807   | 17.667
Mutable std       | 87.903   | 82.112   | 63.517   | 27.870   | 11.431
**deque**         |||||
Immutable cefal   | 113.504  | 192.596  | 307.052  | 447.215  | 568.712
Immutable std     | 600.247  | 595.376  | 578.780  | 616.451  | 637.237
Mutable cefal     | 94.743   | 93.830   | 76.620   | 55.575   | 45.611
Mutable std       | 95.801   | 95.756   | 78.739   | 57.982   | 45.990
**set**           |||||
Immutable cefal   | 8.324    | 20.030   | 37.048  | 61.184    | 72.709
Immutable std     | 69.813   | 65.672   | 68.235  | 61.109    | 57.16
Mutable cefal     | 11.708   | 9.690    | 6.233   | 3.195     | 2.093
Mutable std       | 11.800   | 9.434    | 6.350   | 3.499     | 2.045
**unordered_set** |||||
Immutable cefal   | 4.628    | 7.119    | 10.938  | 15.011    | 19.349
Immutable std     | 19.871   | 21.082   | 23.166  | 17.981    | 16.470
Mutable cefal     | 9.700    | 7.933    | 4.416   | 2.516     | 1.237
Mutable std       | 9.565    | 7.638    | 4.431   | 2.491     | 1.181

### filter() for Expensive
Type              | 10%      | 25%      | 50%      | 75%      | 90%
----------------- | -------: | -------: | -------: | -------: | ---------:
**vector**        |||||
Immutable cefal   | 11.792   | 35.182   | 82.349   | 148.333  | 390.113
Immutable std     | 59.871   | 73.166   | 83.444   | 102.425  | 228.070
Mutable cefal     | 29.069   | 21.547   | 10.397   | 3.508    | 0.702
Mutable std       | 29.330   | 21.699   | 10.461   | 3.786    | 0.764
**list**          |||||
Immutable cefal   | 33.141   | 88.852   | 200.685  | 357.032  | 494.956
Immutable std     | 330.141  | 358.375  | 401.921  | 473.949  | 550.646
Mutable cefal     | 40.624   | 28.490   | 13.667   | 4.227    | 0.892
Mutable std       | 44.192   | 38.115   | 27.135   | 15.882   | 7.012
**deque**         |||||
Immutable cefal   | 58.849   | 170.306  | 417.973  | 660.936  | 782.793
Immutable std     | 603.614  | 703.117  | 848.217  | 867.815  | 875.591
Mutable cefal     | 37.555   | 25.851   | 11.376   | 3.446    | 0.772
Mutable std       | 37.160   | 25.373   | 11.433   | 3.412    | 0.704
**set**           |||||
Immutable cefal   | 109.408  | 270.070  | 579.415  | 842.437  | 1014.060
Immutable std     | 1062.730 | 973.881  | 1292.680 | 1425.520 | 1353.210
Mutable cefal     | 48.569   | 40.696   | 30.280   | 16.878   | 7.575
Mutable std       | 48.070   | 42.310   | 30.579   | 16.679   | 7.590
**unordered_set** |||||
Immutable cefal   | 97.620   | 261.067  | 563.971  | 901.342  | 1011.660
Immutable std     | 863.308  | 841.851  | 884.763  | 883.926  | 873.447
Mutable cefal     | 175.319  | 172.565  | 129.350  | 66.621   | 26.215
Mutable std       | 174.427  | 160.743  | 129.702  | 67.049   | 26.729

### Observations on benchmark results
 * Immutable `map()` is on par with `std::transform`
 * Mutable `map()` for ints is on par, but for move-efficient types it is A LOT faster than `std::transform` (numbers in table above are not a mistake)
 * There is no efficient mutable `std::transform`, but `map()` for ints is roughly the same as immutable versions of both cefal and std
 * Mutable `map()` for move-efficient types is much faster than immutable cefal or std performance
 * `filter()` is not that great as `std::erase_if` for mutable lists of small types (in all cases) and for vectors of heavy types in case when almost whole container is accepted
 * `filter()` is better than `std::erase_if` for mutable lists of move-efficient types, especially when more than half of elements are accepted (more accepted - bigger the gap in favor of `filter()`)
 * `filter()` is on par with `std::erase_if` in case of immutable vector of small types and in case of all other mutable containers not mentioned above
 * For immutable containers `filter()` is either on par or better than `std::erase_if`. Less elements are accepted - bigger the gap for in favor of `filter()` (up to 10x in case of 10% elements accepted)
 * Benchmarks for mapping to another inner type also exist in source codes (not added here for brevity). For immutable containers they show pretty much the same results (i.e. almost equal between cefal and std). Mutable cefal benchmarks for move-effective types though shows 1.5x-2x better performance on all containers except unordered_set.

As a general conclusion: there are for sure few cases where cefal shows itself worse than direct usage of std algorithm (not tremendously though), but there are also a lot of cases where cefal works faster by 1-2 orders of magnitude (especially in case of move-efficient types) and in remaining cases it is on par with std.

Cefal still lacks laziness though, but it can be fixed with ranges coming to std libraries.

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
