#include "cefal/everything.h"

#include <cefal/helpers/std_containers.h>

#include <iostream>
#include <set>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <ranges>

using namespace cefal;

template <concepts::Monoid T>
struct ImplementedMethods {
    T value;
    static ImplementedMethods unit(T x) { return {x}; }
    static ImplementedMethods empty() { return {T()}; }

    auto append(ImplementedMethods other) const {
        return ImplementedMethods{value | ops::append(other.value)};
    };

    auto append(helpers::SingletonFrom<ImplementedMethods> other) const {
        return ImplementedMethods{value | ops::append(other.value)};
    };

    template <typename Result, typename Func>
    auto foldLeft(Result&& initial, Func func) const& {
        return func(std::forward<Result>(initial), value);
    };

    template <typename Result, typename Func>
    auto foldLeft(Result&& initial, Func func) && {
        return func(std::forward<Result>(initial), std::move(value));
    };

    template <typename Func>
    auto filter(Func f) const {
        return ImplementedMethods{value};
    };

    template <typename Func>
    auto map(Func f) const {
        return ImplementedMethods{f(value)};
    };

    template <typename Func>
    auto flatMap(Func f) const {
        return ImplementedMethods{f(value).value};
    };
};

namespace cefal::helpers {
template <typename Src>
struct SingletonFrom<ImplementedMethods<Src>> {
    Src value;
};
}

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
    auto filter = ops::filter([](double x) { return x > 4; });
    auto intermediate = std::forward<M>(m) | ops::map([](int x) { return x + 5; })
                                           | flatMapper
                                           | ops::flatMap([](auto) { return CleanM{4, 5}; })
                                           | std::move(filter);
    auto final = std::move(rounder)(intermediate);
    return *final.begin();
}

double testOptional(const std::optional<int>& opt) {
    auto result = opt | ops::map([](auto x) {return x * 2;})
                      | ops::flatMap([](auto x) -> std::optional<double> {if (x < 5) return std::nullopt; return x;})
                      | ops::map([](auto x) {return x * 3;})
                      | ops::filter([](auto x) {return true;});
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
    std::multimap<std::string, int> m {{"12", 34}, {"56", 78}};
    std::string mapResult1 = m | ops::foldLeft(std::string(), [](std::string c, auto&& x) {
                                    return std::move(c) + std::get<0>(x) + std::to_string(std::get<1>(x)) + " | ";
                                 });
    std::cout << mapResult1 << std::endl;

    std::multimap<std::string, int> mSum = m | ops::append(m);

    std::string mapResult = m | ops::map([](auto&& x) {return std::make_pair(std::get<0>(x), std::to_string(std::get<1>(x)));})
                              | ops::foldLeft(std::string(), [](std::string c, auto&& x) {
                                    return std::move(c) + std::get<0>(x) + std::get<1>(x) + " | ";
                                });
    std::cout << mapResult << std::endl;

    mapResult = std::move(m) | ops::map([](auto&& x) {return std::make_pair(std::get<0>(x), std::to_string(std::get<1>(x)));})
                             | ops::foldLeft(std::string(), [](std::string c, auto&& x) {
                                    return std::move(c) + std::get<0>(x) + std::get<1>(x) + " | ";
                               });
    std::cout << mapResult << std::endl;

    std::set<int> vec = {1, 2, 3};
    for (int x : std::views::all(vec))
        std::cout << x << " ";
    std::cout << std::endl;
    std::string rangeResult = std::views::all(vec) | ops::map([](int x) {return x + 2;})
                                                   | ops::filter([](int x) {return x % 2;})
                                                   | ops::foldLeft(std::string(), [](std::string c, int x) {
                                                        return std::move(c) + std::to_string(x);
                                                     });
    std::cout << rangeResult << std::endl;

    {
        std::set<int> converted = std::vector{1, 2, 3} | ops::as<std::set<int>>();
        std::cout << "1->1 rvalue converted: " << converted.size() << ": ";
        for (auto&& x : converted) std::cout << x << " ";
        std::cout << std::endl;

        auto convertSrc = std::vector{1, 2, 3};
        std::set<int> lvalueConverted = convertSrc | ops::as<std::set>();
        std::cout << "1->1 lvalue converted: " << lvalueConverted.size() << ": ";
        for (auto&& x : lvalueConverted) std::cout << x << " ";
        std::cout << std::endl;

        std::set<double> viewConverted = convertSrc | std::views::transform([](auto x) {return x * 2.5;}) | ops::as<std::set>();
        std::cout << "1->1 view converted: " << viewConverted.size() << ": ";
        for (auto&& x : viewConverted) std::cout << x << " ";
        std::cout << std::endl;
    }

    {
        std::vector<std::pair<std::string, int>> converted = std::map<std::string, int>{{"12", 34}, {"56", 78}} | ops::as<std::vector>();
        std::cout << "2->1 rvalue converted: " << converted.size() << ": ";
        for (auto&& [x, y] : converted) std::cout << x << ":" << y << " ";
        std::cout << std::endl;

        auto convertSrc = std::map<std::string, int> {{"12", 34}, {"56", 78}};
        std::vector<std::pair<std::string, int>> lvalueConverted = convertSrc | ops::as<std::vector>();
        std::cout << "2->1 lvalue converted: " << lvalueConverted.size() << ": ";
        for (auto&& [x, y] : lvalueConverted) std::cout << x << ":" << y << " ";
        std::cout << std::endl;

        std::set<std::pair<std::string, double>> viewConverted =
            convertSrc | std::views::transform([](auto x) {return std::make_pair(std::get<0>(x), std::get<1>(x) * 2.5);})
                       | ops::as<std::set>();
        std::cout << "2->1 view converted: " << viewConverted.size() << ": ";
        for (auto&& [x, y] : viewConverted) std::cout << x << ":" << y << " ";
        std::cout << std::endl;
    }

    {
        std::unordered_map<std::string, int> converted = std::map<std::string, int>{{"12", 34}, {"56", 78}} | ops::as<std::unordered_map>();
        std::cout << "2->2 rvalue converted: " << converted.size() << ": ";
        for (auto&& [x, y] : converted) std::cout << x << ":" << y << " ";
        std::cout << std::endl;

        auto convertSrc = std::map<std::string, int> {{"12", 34}, {"56", 78}};
        std::unordered_map<std::string, int> lvalueConverted = convertSrc | ops::as<std::unordered_map>();
        std::cout << "2->2 lvalue converted: " << lvalueConverted.size() << ": ";
        for (auto&& [x, y] : lvalueConverted) std::cout << x << ":" << y << " ";
        std::cout << std::endl;

        std::unordered_map<std::string, double> viewConverted =
            convertSrc | std::views::transform([](auto x) {return std::make_pair(std::get<0>(x), std::get<1>(x) * 2.5);})
                       | ops::as<std::unordered_map>();
        std::cout << "2->2 view converted: " << viewConverted.size() << ": ";
        for (auto&& [x, y] : viewConverted) std::cout << x << ":" << y << " ";
        std::cout << std::endl;
    }

    {
        std::unordered_map<std::string, int> converted = std::vector<std::pair<std::string, int>>{{"12", 34}, {"56", 78}} | ops::as<std::unordered_map>();
        std::cout << "1t->2 rvalue converted: " << converted.size() << ": ";
        for (auto&& [x, y] : converted) std::cout << x << ":" << y << " ";
        std::cout << std::endl;

        auto convertSrc = std::vector<std::pair<std::string, int>> {{"12", 34}, {"56", 78}};
        std::unordered_map<std::string, int> lvalueConverted = convertSrc | ops::as<std::unordered_map>();
        std::cout << "1t->2 lvalue converted: " << lvalueConverted.size() << ": ";
        for (auto&& [x, y] : lvalueConverted) std::cout << x << ":" << y << " ";
        std::cout << std::endl;

        std::unordered_map<std::string, double> viewConverted =
            convertSrc | std::views::transform([](auto x) {return std::make_pair(std::get<0>(x), std::get<1>(x) * 2.5);})
                       | ops::as<std::unordered_map>();
        std::cout << "1t->2 view converted: " << viewConverted.size() << ": ";
        for (auto&& [x, y] : viewConverted) std::cout << x << ":" << y << " ";
        std::cout << std::endl;
    }

    {
        std::unordered_map<std::string, int> converted = std::vector<std::pair<std::string, int>>{{"12", 34}, {"56", 78}} | ops::as<std::unordered_map>();
        std::cout << "1p->2 rvalue converted: " << converted.size() << ": ";
        for (auto&& [x, y] : converted) std::cout << x << ":" << y << " ";
        std::cout << std::endl;

        auto convertSrc = std::vector<std::pair<std::string, int>> {{"12", 34}, {"56", 78}};
        std::unordered_map<std::string, int> lvalueConverted = convertSrc | ops::as<std::unordered_map>();
        std::cout << "1p->2 lvalue converted: " << lvalueConverted.size() << ": ";
        for (auto&& [x, y] : lvalueConverted) std::cout << x << ":" << y << " ";
        std::cout << std::endl;

        std::unordered_map<std::string, double> viewConverted =
            convertSrc | std::views::transform([](auto x) {return std::make_pair(std::get<0>(x), std::get<1>(x) * 2.5);})
                       | ops::as<std::unordered_map>();
        std::cout << "1p->2 view converted: " << viewConverted.size() << ": ";
        for (auto&& [x, y] : viewConverted) std::cout << x << ":" << y << " ";
        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << getInt(42) << std::endl;
    std::cout << testOptional(3) << "; " <<  testOptional(2) << "; " <<  testOptional(std::nullopt) << std::endl;
    { ContainerTester<std::vector<int>> x; }
    { ContainerTester<std::vector<double>> x; }
    { ContainerTester<std::set<int>> x; }
    { ContainerTester<std::set<double>> x; }
    { ContainerTester<std::unordered_set<int>> x; }
    { ContainerTester<std::unordered_set<double>> x; }
    std::vector<std::vector<int>> innerMapped = std::vector{1, 2, 3} | ops::map([](int x) { return ops::unit<std::vector>(x); })
                                                                     | ops::innerFilter([](int x){ return x % 2; })
                                                                     | ops::innerFlatMap([](int x){ return std::vector{x + 1, x + 2}; })
                                                                     | ops::innerMap([](int x){ return x * 3; });
    std::cout << "innerMapped: " << innerMapped.size() << ": | ";
    for (auto&& inner : innerMapped) {
        for (int x : inner)
            std::cout << x << " ";
        std::cout << "| ";
    }
    std::cout << std::endl;

    ops::empty<ImplementedMethods<Sum<int>>>() | ops::foldLeft(0, [](int a, Sum<int> b) {return a + b.value;});
    ops::unit<ImplementedMethods<Sum<int>>>(Sum(42));
    ImplementedMethods<Sum<int>> impl =
        ops::unit<ImplementedMethods>(Sum(42)) | ops::map([](int x) -> Sum<int> {return x + 2;})
                                               | ops::filter([](auto x) {return true;})
                                               | ops::flatMap([](int x) {return ImplementedMethods<Sum<int>>{x * 3};});
    std::cout << impl.value.value << std::endl;

    SimpleImplementedMethods nonTemplatedImpl =
        ops::unit<SimpleImplementedMethods>(42) | ops::map([](int x) {return x + 2;})
                                                | ops::flatMap([](int x) {return SimpleImplementedMethods{x * 3};});
    std::cout << nonTemplatedImpl.value << std::endl;
}
