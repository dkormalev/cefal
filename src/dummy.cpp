#include "cefal/cefal"

#include <iostream>
#include <set>
#include <unordered_set>
#include <vector>

using namespace cefal;

template <concepts::Monad M>
int test(const M& m) {
    return *(m | ops::flatMap([](auto) { return std::vector<double>{4.5, 5.5}; })
               | ops::flatMap([](auto) { return std::set<double>{4.5, 5.5}; })
               | ops::map([](double x) { return int(x); }))
                .begin();
}

int getInt(int seed) {
    return test(ops::unit<std::set<int>>(seed));
}

template <concepts::Functor C>
struct Tester {
    Tester() {
        C a = ops::unit<C>(5);
        test(a);
    }
};

int main() {
    std::cout << getInt(42) << std::endl;
    { Tester<std::vector<int>> x; }
    { Tester<std::vector<double>> x; }
    { Tester<std::set<int>> x; }
    { Tester<std::set<double>> x; }
    { Tester<std::unordered_set<int>> x; }
    { Tester<std::unordered_set<double>> x; }
    // { Tester<std::map<int, int>> x; } // should fail
}
