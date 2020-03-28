#include "cefal/cefal"
#include "cefal/instances/monoid/basic_types.h"
#include "cefal/instances/monoid/std_containers.h"
#include "cefal/instances/monoid/with_functions.h"
#include "cefal/instances/filterable/from_foldable.h"
#include "cefal/instances/filterable/with_functions.h"
#include "cefal/instances/filterable/std_optional.h"
#include "cefal/instances/foldable/std_containers.h"
#include "cefal/instances/foldable/with_functions.h"
#include "cefal/instances/functor/from_foldable.h"
#include "cefal/instances/functor/with_functions.h"
#include "cefal/instances/functor/std_optional.h"
#include "cefal/instances/monad/from_foldable.h"
#include "cefal/instances/monad/with_functions.h"
#include "cefal/instances/monad/std_optional.h"

#include "tests/counter.h"

#include <iostream>
#include <set>
#include <unordered_set>
#include <vector>
#include <chrono>


std::atomic_uint64_t Counter::_created{0};
std::atomic_uint64_t Counter::_copied{0};
std::atomic_uint64_t Counter::_moved{0};
std::atomic_uint64_t Counter::_deleted{0};

std::mutex Counter::customMutex{};
std::unordered_map<std::string, uint64_t> Counter::_custom{};

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

int storage = 0;

template<typename InnerSrc, typename InnerDest>
void vectorsBenchmark(int repeats = 10, size_t size = 10'000'000) {
    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            std::vector<InnerSrc> src;
            src.reserve(size);
            for (int j = 0; j < size; ++j) src.push_back(start2.time_since_epoch().count() + j);

            std::vector<InnerDest> dest;
            dest.reserve(size);
            std::transform(src.begin(), src.end(), std::back_inserter(dest), [](const InnerSrc& x) -> InnerDest {return x / 2.0; });
            storage = *src.begin();
        }
        auto elapsed = std::chrono::system_clock::now() - start;
    }

    Counter::reset();
    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            std::vector<InnerSrc> src;
            src.reserve(size);
            for (int j = 0; j < size; ++j) src.push_back(start2.time_since_epoch().count() + j);

            std::vector<InnerDest> dest;
            std::transform(src.begin(), src.end(), std::back_inserter(dest), [](const InnerSrc& x) -> InnerDest {return x / 2.0; });
            storage = *dest.begin();
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        std::cout << "Transform         = " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed / repeats).count() << std::endl;
    }
    // std::cout << "N: " << Counter::created() << "; C: " << Counter::copied() << "; M: " << Counter::moved() << std::endl;
    // Counter::reset();
    // std::cout << storage << std::endl;

    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            std::vector<InnerSrc> src;
            src.reserve(size);
            for (int j = 0; j < size; ++j) src.push_back(start2.time_since_epoch().count() + j);

            std::vector<InnerDest> dest = src | ops::map([](const InnerSrc& x) -> InnerDest {return x / 2.0; });
            storage = *dest.begin();
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        std::cout << "Map     Immutable = " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed / repeats).count() << std::endl;
    }
    // std::cout << "N: " << Counter::created() << "; C: " << Counter::copied() << "; M: " << Counter::moved() << std::endl;
    // Counter::reset();
    // std::cout << storage << std::endl;

    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            std::vector<InnerSrc> src;
            src.reserve(size);
            for (int j = 0; j < size; ++j) src.push_back(start2.time_since_epoch().count() + j);

            std::vector<InnerDest> dest;
            dest.reserve(size);
            std::transform(src.begin(), src.end(), std::back_inserter(dest), [](const InnerSrc& x) -> InnerDest {return x / 2.0; });
            storage = *dest.begin();
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        std::cout << "Transform/reserve = " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed / repeats).count() << std::endl;
    }
    // std::cout << "N: " << Counter::created() << "; C: " << Counter::copied() << "; M: " << Counter::moved() << std::endl;
    // Counter::reset();
    // std::cout << storage << std::endl;

    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            std::vector<InnerSrc> src;
            src.reserve(size);
            for (int j = 0; j < size; ++j) src.push_back(start2.time_since_epoch().count() + j);

            src = std::move(src) | ops::map([](InnerSrc&& x) {return std::move(x) / 2; });
            storage = *src.begin();
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        std::cout << "Map       Mutable = " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed / repeats).count() << std::endl;
    }
    // std::cout << "N: " << Counter::created() << "; C: " << Counter::copied() << "; M: " << Counter::moved() << std::endl;
    // Counter::reset();
    // std::cout << storage << std::endl;

    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            std::vector<InnerSrc> src;
            src.reserve(size);
            for (int j = 0; j < size; ++j) src.push_back(start2.time_since_epoch().count() + j);

            std::transform(src.begin(), src.end(), src.begin(), [](const InnerSrc& x) {return x / 2; });
            storage = *src.begin();
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        std::cout << "Transform/self    = " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed / repeats).count() << std::endl;
    }
    // std::cout << "N: " << Counter::created() << "; C: " << Counter::copied() << "; M: " << Counter::moved() << std::endl;
    // Counter::reset();
    // std::cout << storage << std::endl;

    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            std::vector<InnerSrc> src;
            src.reserve(size);
            for (int j = 0; j < size; ++j) src.push_back(start2.time_since_epoch().count() + j);

            auto dest = src | ops::filter([](const InnerSrc& x) {return x % 2; });
            storage = *dest.begin();
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        std::cout << "Filter  Immutable = " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed / repeats).count() << std::endl;
    }
    // std::cout << "N: " << Counter::created() << "; C: " << Counter::copied() << "; M: " << Counter::moved() << std::endl;
    // Counter::reset();
    // std::cout << storage << std::endl;

    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            std::vector<InnerSrc> src;
            src.reserve(size);
            for (int j = 0; j < size; ++j) src.push_back(start2.time_since_epoch().count() + j);

            auto dest = src;
            std::erase_if(dest, [](const InnerSrc& x) {return !(x % 2); });
            storage = *dest.begin();
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        std::cout << "EraseIf Immutable = " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed / repeats).count() << std::endl;
    }
    // std::cout << "N: " << Counter::created() << "; C: " << Counter::copied() << "; M: " << Counter::moved() << std::endl;
    // Counter::reset();
    // std::cout << storage << std::endl;

    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            std::vector<InnerSrc> src;
            src.reserve(size);
            for (int j = 0; j < size; ++j) src.push_back(start2.time_since_epoch().count() + j);

            src = std::move(src) | ops::filter([](const InnerSrc& x) {return x % 2; });
            storage = *src.begin();
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        std::cout << "Filter    Mutable = " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed / repeats).count() << std::endl;
    }
    // std::cout << "N: " << Counter::created() << "; C: " << Counter::copied() << "; M: " << Counter::moved() << std::endl;
    // Counter::reset();
    // std::cout << storage << std::endl;

    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            std::vector<InnerSrc> src;
            src.reserve(size);
            for (int j = 0; j < size; ++j) src.push_back(start2.time_since_epoch().count() + j);

            std::erase_if(src, [](const InnerSrc& x) {return !(x % 2); });
            storage = *src.begin();
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        std::cout << "EraseIf   Mutable = " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed / repeats).count() << std::endl;
    }
    // std::cout << "N: " << Counter::created() << "; C: " << Counter::copied() << "; M: " << Counter::moved() << std::endl;
    // Counter::reset();
    // std::cout << storage << std::endl;
}

template<template <typename...> typename T, typename InnerSrc, typename InnerDest>
void setsBenchmark(int repeats = 10, size_t size = 100'000) {
    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            T<InnerSrc> src;
            for (int j = 0; j < size; ++j) src.insert(start2.time_since_epoch().count() + j);

            T<InnerDest> dest;
            std::transform(src.begin(), src.end(), std::inserter(dest, dest.end()), [](const InnerSrc& x) -> InnerDest {return x / 2.0; });
        }
        auto elapsed = std::chrono::system_clock::now() - start;
    }

    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            T<InnerSrc> src;
            for (int j = 0; j < size; ++j) src.insert(start2.time_since_epoch().count() + j);

            T<InnerDest> dest = src | ops::map([](const InnerSrc& x) -> InnerDest {return x / 2.0; });
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        std::cout << "Map     Immutable = " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed / repeats).count() << std::endl;
    }

    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            T<InnerSrc> src;
            for (int j = 0; j < size; ++j) src.insert(start2.time_since_epoch().count() + j);

            T<InnerDest> dest;
            std::transform(src.begin(), src.end(), std::inserter(dest, dest.end()), [](const InnerSrc& x) -> InnerDest {return x / 2.0; });
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        std::cout << "Transform         = " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed / repeats).count() << std::endl;
    }

    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            T<InnerSrc> src;
            for (int j = 0; j < size; ++j) src.insert(start2.time_since_epoch().count() + j);

            src = std::move(src) | ops::map([](InnerSrc&& x) {return std::move(x) / 2; });
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        std::cout << "Map       Mutable = " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed / repeats).count() << std::endl;
    }

    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            T<InnerSrc> src;
            for (int j = 0; j < size; ++j) src.insert(start2.time_since_epoch().count() + j);

            auto dest = src | ops::filter([](const InnerSrc& x) {return x % 2; });
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        std::cout << "Filter  Immutable = " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed / repeats).count() << std::endl;
    }

    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            T<InnerSrc> src;
            for (int j = 0; j < size; ++j) src.insert(start2.time_since_epoch().count() + j);

            auto dest = src;
            std::erase_if(dest, [](const InnerSrc& x) {return !(x % 2); });
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        std::cout << "EraseIf Immutable = " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed / repeats).count() << std::endl;
    }

    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            T<InnerSrc> src;
            for (int j = 0; j < size; ++j) src.insert(start2.time_since_epoch().count() + j);

            src = std::move(src) | ops::filter([](const InnerSrc& x) {return x % 2; });
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        std::cout << "Filter    Mutable = " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed / repeats).count() << std::endl;
    }

    {
        auto start = std::chrono::system_clock::now();
        for (int i = 0; i < repeats; ++i) {
            auto start2 = std::chrono::system_clock::now();
            T<InnerSrc> src;
            for (int j = 0; j < size; ++j) src.insert(start2.time_since_epoch().count() + j);

            std::erase_if(src, [](const InnerSrc& x) {return !(x % 2); });
        }
        auto elapsed = std::chrono::system_clock::now() - start;
        std::cout << "EraseIf   Mutable = " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed / repeats).count() << std::endl;
    }
}

struct HeavyPayload : Counter {
    uint64_t unused[100];
};

template <typename T>
struct Heavy {
    Heavy(T x) : value(std::move(x)) {}
    Heavy(T x, HeavyPayload payload) : value(std::move(x)), payload(std::move(payload)) {}
    Heavy(const Heavy&) = default;
    Heavy(Heavy&&) = default;
    Heavy& operator=(const Heavy&) = default;
    Heavy& operator=(Heavy&&) = default;
    ~Heavy() = default;

    operator T() const {
        return value;
    }

    template <typename U>
    bool operator%(U&& other) const { return value % other; }

    template <typename U>
    Heavy<std::remove_cvref_t<U>> operator/(U&& other) const& { return Heavy<std::remove_cvref_t<U>>(value / other); }

    template <typename U>
    Heavy<std::remove_cvref_t<U>> operator/(U&& other) && {
        if constexpr (std::is_same_v<std::remove_cvref_t<U>, T>) {
            value /= other;
            return std::move(*this);
        } else {
            return Heavy<std::remove_cvref_t<U>>(value / other, std::move(payload));
        }
    }

    auto operator<=> (const Heavy<T>& other) const { return value <=> other.value; }
    bool operator== (const Heavy<T>& other) const { return std::abs(value - other.value) < 0.0001; }

    HeavyPayload payload;
    T value = T();
};


template <typename T>
struct Expensive {
    Expensive(T x) : value(std::move(x)), payload(new char[102400]) {}
    Expensive(T x, char* payload) : value(std::move(x)), payload(payload) {}
    Expensive(const Expensive& other) {
        payload = new char[102400];
        value = other.value;
    }
    Expensive(Expensive&& other) {
        payload = other.payload;
        value = std::move(other.value);
        other.payload = 0;
    }
    Expensive& operator=(const Expensive& other) {
        payload = new char[102400];
        value = other.value;
        return *this;
    }
    Expensive& operator=(Expensive&& other) {
        payload = other.payload;
        value = std::move(other.value);
        other.payload = 0;
        return *this;
    }
    ~Expensive() {delete[] payload;}

    operator T() const {
        return value;
    }

    template <typename U>
    bool operator%(U&& other) const { return value % other; }

    template <typename U>
    Expensive<std::remove_cvref_t<U>> operator/(U&& other) const& { return Expensive<std::remove_cvref_t<U>>(value / other); }

    template <typename U>
    Expensive<std::remove_cvref_t<U>> operator/(U&& other) && {
        if constexpr (std::is_same_v<std::remove_cvref_t<U>, T>) {
            value /= other;
            return std::move(*this);
        } else {
            auto result = Expensive<std::remove_cvref_t<U>>(value / other, std::move(payload));
            payload = 0;
            return result;
        }
    }

    auto operator<=> (const Expensive<T>& other) const { return value <=> other.value; }
    bool operator== (const Expensive<T>& other) const { return std::abs(value - other.value) < 0.0001; }

    char* payload;
    T value = T();
};

namespace std {
    template<typename T>
    struct hash<Heavy<T>> {
        size_t operator()(const Heavy<T>& x) const { return std::hash<T>()(x.value); }
    };

    template<typename T>
    struct hash<Expensive<T>> {
        size_t operator()(const Expensive<T>& x) const { return std::hash<T>()(x.value); }
    };
}

int main() {
    std::cout << "\nstd::vector<int>:" << std::endl;
    vectorsBenchmark<int, double>(10, 10'000'000);
    std::cout << "\nstd::set<int>:" << std::endl;
    setsBenchmark<std::set, int, double>(10, 1'000'000);
    std::cout << "\nstd::unordered_set<int>:" << std::endl;
    setsBenchmark<std::unordered_set, int, double>(10, 1'000'000);

    std::cout << "\nstd::vector<Heavy>:" << std::endl;
    vectorsBenchmark<Heavy<int>, Heavy<double>>(10, 500'000);
    std::cout << "\nstd::set<Heavy>:" << std::endl;
    setsBenchmark<std::set, Heavy<int>, Heavy<double>>(10, 100'000);
    std::cout << "\nstd::unordered_set<Heavy>:" << std::endl;
    setsBenchmark<std::unordered_set, Heavy<int>, Heavy<double>>(10, 100'000);

    std::cout << "\nstd::vector<Expensive>:" << std::endl;
    vectorsBenchmark<Expensive<int>, Expensive<double>>(10, 50'000);
    std::cout << "\nstd::set<Expensive>:" << std::endl;
    setsBenchmark<std::set, Expensive<int>, Expensive<double>>(10, 50'000);
    std::cout << "\nstd::unordered_set<Expensive>:" << std::endl;
    setsBenchmark<std::unordered_set, Expensive<int>, Expensive<double>>(10, 50'000);

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
