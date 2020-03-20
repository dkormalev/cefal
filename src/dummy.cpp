#include "cefal/instances/foldable/std_containers.h"
#include "cefal/instances/monoid/std_containers.h"
#include "cefal/instances/functor/from_foldable.h"
#include "cefal/instances/monad/from_foldable.h"
#include "cefal/instances/functor/with_functions.h"
#include "cefal/instances/monad/with_functions.h"
#include "cefal/instances/functor/std_optional.h"
#include "cefal/instances/monad/std_optional.h"

#include <iostream>
#include <set>
#include <unordered_set>
#include <vector>

using namespace cefal;

template <typename T>
struct ImplementedMethods {
    T value;
    static ImplementedMethods unit(T x) { return {x}; }

    template <typename Func>
    auto map(Func f) const {
        return ImplementedMethods{f(value)};
    };

    template <typename Func>
    auto flatMap(Func f) const {
        return ImplementedMethods{f(value).value};
    };
};

struct SimpleImplementedMethods {
    int value;
    static SimpleImplementedMethods unit(int x) { return {x}; }

    template <typename Func>
    auto map(Func f) const {
        return SimpleImplementedMethods{f(value)};
    };

    template <typename Func>
    auto flat_map(Func f) const & {
        return SimpleImplementedMethods{f(value).value};
    };

    template <typename Func>
    auto flat_map(Func f) && {
        return SimpleImplementedMethods{f(value).value};
    };
};

namespace cefal {
template <>
struct InnerType<SimpleImplementedMethods> {
    using type = int;
};
template <>
struct WithInnerType<SimpleImplementedMethods, int> {
    using type = SimpleImplementedMethods;
};
}

template <concepts::Monad M>
int testContainers(M&& m) {
    using CleanM = std::remove_cvref_t<M>;
    auto flatMapper = ops::flatMap([](auto) { return CleanM{4, 5}; });
    auto rounder = ops::map([](double x) { return int(x); });
    auto intermediate = std::forward<M>(m) | ops::map([](int x) { return x + 5; })
                                           | flatMapper
                                           | ops::flatMap([](auto) { return CleanM{4, 5}; });
    auto final = std::move(rounder)(intermediate);
    return *final.begin();
}

double testOptional(const std::optional<int>& opt) {
    auto result = opt | ops::map([](auto x) {return x * 2;})
                      | ops::flatMap([](auto x) -> std::optional<double> {if (x < 5) return std::nullopt; return x;})
                      | ops::map([](auto x) {return x * 3;});
    return result ? *result : -1;
}

int getInt(int seed) {
    return testContainers(ops::unit<std::set<int>>(seed));
}

template <concepts::Functor C>
struct ContainerTester {
    ContainerTester() {
        C a = ops::unit<C>(5);
        testContainers(a);
        testContainers(std::move(a));
    }
};

int main() {
    std::cout << getInt(42) << std::endl;
    std::cout << testOptional(3) << "; " <<  testOptional(2) << "; " <<  testOptional(std::nullopt) << std::endl;
    { ContainerTester<std::vector<int>> x; }
    { ContainerTester<std::vector<double>> x; }
    { ContainerTester<std::set<int>> x; }
    { ContainerTester<std::set<double>> x; }
    { ContainerTester<std::unordered_set<int>> x; }
    { ContainerTester<std::unordered_set<double>> x; }
    std::vector<std::vector<int>> innerMapped = std::vector{1, 2, 3} | ops::map([](int x) { return ops::unit<std::vector>(x); })
                                                                     | ops::innerFlatMap([](int x){ return std::vector{x + 1, x + 2}; })
                                                                     | ops::innerMap([](int x){ return x * 3; });
    std::cout << "innerMapped: " << innerMapped.size() << ": | ";
    for (auto&& inner : innerMapped) {
        for (int x : inner)
            std::cout << x << " ";
        std::cout << "| ";
    }
    std::cout << std::endl;

    ImplementedMethods<int> impl =
        ops::unit<ImplementedMethods>(42) | ops::map([](int x) {return x + 2;})
                                          | ops::flatMap([](int x) {return ImplementedMethods<int>{x * 3};});
    std::cout << impl.value << std::endl;

    SimpleImplementedMethods nonTemplatedImpl =
        ops::unit<SimpleImplementedMethods>(42) | ops::map([](int x) {return x + 2;})
                                                | ops::flatMap([](int x) {return SimpleImplementedMethods{x * 3};});
    std::cout << nonTemplatedImpl.value << std::endl;
}
