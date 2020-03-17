#include "cefal/cefal"

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


template <concepts::Monad M>
int testContainers(M&& m) {
    return *(std::forward<M>(m) | ops::map([](int x) { return x + 5; })
                                | ops::flatMap([](auto) { return std::vector<double>{4.5, 5.5}; })
                                | ops::flatMap([](auto) { return std::set<double>{4.5, 5.5}; })
                                | ops::map([](double x) { return int(x); }))
                                 .begin();
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
    std::vector<int> united = ops::unit<std::vector>(5);

    ImplementedMethods<int> impl =
        ops::unit<ImplementedMethods>(42) | ops::map([](int x) {return x + 2;})
                                          | ops::flatMap([](int x) {return ImplementedMethods<int>{x * 3};});
    std::cout << impl.value << std::endl;
}
