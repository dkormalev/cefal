#include "cefal/cefal"

#include <iostream>
#include <set>
#include <unordered_set>
#include <vector>

using namespace cefal;

template <concepts::Monad M>
int testContainers(const M& m) {
    return *(m | ops::flatMap([](auto) { return std::vector<double>{4.5, 5.5}; })
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
    // { Tester<std::map<int, int>> x; } // should fail
}
