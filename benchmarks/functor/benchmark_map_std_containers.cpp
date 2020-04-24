/* Copyright 2020, Dennis Kormalev
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *     * Neither the name of the copyright holders nor the names of its contributors may be used to
 * endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "expensive.h"

#include "cefal/everything.h"

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch2/catch.hpp"

#include <deque>
#include <iostream>
#include <list>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

using namespace cefal;

template <typename T>
struct ContainerSize;

template <typename T>
struct ContainerSize<std::vector<T>> {
    static constexpr size_t value = std::is_same_v<T, int> ? 10'000'000 : 25'000;
};

template <typename T>
struct ContainerSize<std::list<T>> {
    static constexpr size_t value = std::is_same_v<T, int> ? 1'000'000 : 25'000;
};

template <typename T>
struct ContainerSize<std::deque<T>> {
    static constexpr size_t value = std::is_same_v<T, int> ? 10'000'000 : 25'000;
};

template <cefal::detail::SetLikeContainer C>
struct ContainerSize<C> {
    static constexpr size_t value = std::is_same_v<cefal::InnerType_T<C>, int> ? 100'000 : 25'000;
};

template <cefal::detail::DoubleSocketedStdContainer C>
struct ContainerSize<C> {
    static constexpr size_t value =
        std::is_same_v<std::remove_cvref_t<std::tuple_element_t<0, cefal::InnerType_T<C>>>,
                       int> && std::is_same_v<std::remove_cvref_t<std::tuple_element_t<1, cefal::InnerType_T<C>>>, int>
            ? 100'000
            : 25'000;
};

template <typename T>
constexpr inline size_t ContainerSize_V = ContainerSize<T>::value;

TEMPLATE_PRODUCT_TEST_CASE("cefal::map() to same type", "", (std::vector, std::list, std::deque), (int, Expensive<int>)) {
    auto func = []<typename T>(T&& x) { return std::forward<T>(x) + 1; };

    BENCHMARK_ADVANCED("cefal::map() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        TestType src;
        for (int j = 0; j < ContainerSize_V<TestType>; ++j)
            src.push_back(seed.time_since_epoch().count() + j);
        meter.measure([&src, &func] {
            TestType dest = src | ops::map(func);
            return *dest.begin();
        });
    };

    BENCHMARK_ADVANCED("std::transform() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        TestType src;
        for (int j = 0; j < ContainerSize_V<TestType>; ++j)
            src.push_back(seed.time_since_epoch().count() + j);
        meter.measure([&src, &func] {
            TestType dest = cefal::instances::detail::createMapDestination<TestType>(src); // calls reserve()
            std::transform(src.begin(), src.end(), std::back_inserter(dest), func);
            return *dest.begin();
        });
    };

    BENCHMARK_ADVANCED("cefal::map() - mutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        TestType src;
        for (int j = 0; j < ContainerSize_V<TestType>; ++j)
            src.push_back(seed.time_since_epoch().count() + j);
        meter.measure([&src, &func](int i) {
            src = std::move(src) | ops::map(func);
            return *src.begin();
        });
    };

    BENCHMARK_ADVANCED("std::transform() - mutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        TestType src;
        for (int j = 0; j < ContainerSize_V<TestType>; ++j)
            src.push_back(seed.time_since_epoch().count() + j);
        meter.measure([&src, &func](int i) {
            std::transform(src.begin(), src.end(), src.begin(), func);
            return *src.begin();
        });
    };
}

TEMPLATE_PRODUCT_TEST_CASE("cefal::map() to same type", "",
                           (std::set, std::unordered_set, std::multiset, std::unordered_multiset), (int, Expensive<int>)) {
    auto func = []<typename T>(T&& x) { return std::forward<T>(x) + 1; };

    BENCHMARK_ADVANCED("cefal::map() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        TestType src;
        for (int j = 0; j < ContainerSize_V<TestType>; ++j)
            src.insert(seed.time_since_epoch().count() + j);
        meter.measure([&src, &func] {
            TestType dest = src | ops::map(func);
            return *dest.begin();
        });
    };

    BENCHMARK_ADVANCED("std::transform() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        TestType src;
        for (int j = 0; j < ContainerSize_V<TestType>; ++j)
            src.insert(seed.time_since_epoch().count() + j);
        meter.measure([&src, &func] {
            TestType dest;
            std::transform(src.begin(), src.end(), std::inserter(dest, dest.end()), func);
            return *dest.begin();
        });
    };

    BENCHMARK_ADVANCED("cefal::map() - mutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        TestType src;
        for (int j = 0; j < ContainerSize_V<TestType>; ++j)
            src.insert(seed.time_since_epoch().count() + j);
        meter.measure([&src, &func](int i) {
            src = std::move(src) | ops::map(func);
            return *src.begin();
        });
    };
}

TEMPLATE_PRODUCT_TEST_CASE("cefal::map() to same type", "", (std::map, std::unordered_map),
                           ((int, int), (int, Expensive<int>), (Expensive<int>, int))) {
    using InnerType = cefal::InnerType_T<TestType>;
    BENCHMARK_ADVANCED("cefal::map() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto func = [](const auto& x) {
            auto result = x;
            result.second += 1;
            return result;
        };
        auto seed = std::chrono::system_clock::now();
        TestType src;
        for (int j = 0; j < ContainerSize_V<TestType>; ++j)
            src[seed.time_since_epoch().count() + j] = seed.time_since_epoch().count() + j + 1;
        meter.measure([&src, &func] {
            TestType dest = src | ops::map(func);
            return dest.begin()->second + 1;
        });
    };

    BENCHMARK_ADVANCED("std::transform() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto func = [](const auto& x) {
            auto result = x;
            result.second += 1;
            return result;
        };
        auto seed = std::chrono::system_clock::now();
        TestType src;
        for (int j = 0; j < ContainerSize_V<TestType>; ++j)
            src[seed.time_since_epoch().count() + j] = seed.time_since_epoch().count() + j + 1;
        meter.measure([&src, &func] {
            TestType dest;
            std::transform(src.begin(), src.end(), std::inserter(dest, dest.end()), func);
            return dest.begin()->second + 1;
        });
    };

    BENCHMARK_ADVANCED("cefal::map() - mutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto func = [](auto&& x) {
            x.second += 1;
            return std::move(x);
        };
        auto seed = std::chrono::system_clock::now();
        TestType src;
        for (int j = 0; j < ContainerSize_V<TestType>; ++j)
            src[seed.time_since_epoch().count() + j] = seed.time_since_epoch().count() + j + 1;
        meter.measure([&src, &func](int i) {
            src = std::move(src) | ops::map(func);
            return src.begin()->second + 1;
        });
    };
}

TEMPLATE_PRODUCT_TEST_CASE("cefal::map() to different type", "", (std::vector, std::list, std::deque), (int, Expensive<int>)) {
    auto func = []<typename T>(T&& x) { return std::forward<T>(x) + (double)1.0; };
    using Dest = WithInnerType_T<TestType, std::invoke_result_t<decltype(func), InnerType_T<TestType>>>;

    BENCHMARK_ADVANCED("cefal::map() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        TestType src;
        for (int j = 0; j < ContainerSize_V<TestType>; ++j)
            src.push_back(seed.time_since_epoch().count() + j);
        meter.measure([&src, &func](int i) {
            auto dest = src | ops::map(func);
            return *dest.begin();
        });
    };

    BENCHMARK_ADVANCED("std::transform() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        TestType src;
        for (int j = 0; j < ContainerSize_V<TestType>; ++j)
            src.push_back(seed.time_since_epoch().count() + j);
        meter.measure([&src, &func](int i) {
            auto dest = cefal::instances::detail::createMapDestination<Dest>(src); // calls reserve()
            std::transform(src.begin(), src.end(), std::back_inserter(dest), func);
            return *dest.begin();
        });
    };

    BENCHMARK_ADVANCED("cefal::map() - mutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> src(meter.runs());
        for (auto&& x : src) {
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.push_back(seed.time_since_epoch().count() + j);
        }
        meter.measure([&src, &func](int i) {
            auto dest = std::move(src[i]) | ops::map(func);
            return *dest.begin();
        });
    };
}

TEMPLATE_PRODUCT_TEST_CASE("cefal::map() to different type", "",
                           (std::set, std::unordered_set, std::multiset, std::unordered_multiset), (int, Expensive<int>)) {
    auto func = []<typename T>(T&& x) { return std::forward<T>(x) + (double)1.0; };
    using Dest = WithInnerType_T<TestType, std::invoke_result_t<decltype(func), InnerType_T<TestType>>>;

    BENCHMARK_ADVANCED("cefal::map() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        TestType src;
        for (int j = 0; j < ContainerSize_V<TestType>; ++j)
            src.insert(seed.time_since_epoch().count() + j);
        meter.measure([&src, &func](int i) {
            auto dest = src | ops::map(func);
            return *dest.begin();
        });
    };

    BENCHMARK_ADVANCED("std::transform() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        TestType src;
        for (int j = 0; j < ContainerSize_V<TestType>; ++j)
            src.insert(seed.time_since_epoch().count() + j);
        meter.measure([&src, &func](int i) {
            Dest dest;
            std::transform(src.begin(), src.end(), std::inserter(dest, dest.end()), func);
            return *dest.begin();
        });
    };

    BENCHMARK_ADVANCED("cefal::map() - mutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> src(meter.runs());
        for (auto&& x : src) {
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.insert(seed.time_since_epoch().count() + j);
        }
        meter.measure([&src, &func](int i) {
            auto dest = std::move(src[i]) | ops::map(func);
            return *dest.begin();
        });
    };
}

TEMPLATE_PRODUCT_TEST_CASE("cefal::map() to different type", "", (std::map, std::unordered_map),
                           ((int, int), (int, Expensive<int>), (Expensive<int>, int))) {
    using InnerType = cefal::InnerType_T<TestType>;
    BENCHMARK_ADVANCED("cefal::map() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto func = [](const auto& x) { return std::make_pair(x.first, x.second + (double)1.0); };
        using Dest = WithInnerType_T<TestType, std::invoke_result_t<decltype(func), InnerType_T<TestType>>>;
        auto seed = std::chrono::system_clock::now();
        TestType src;
        for (int j = 0; j < ContainerSize_V<TestType>; ++j)
            src[seed.time_since_epoch().count() + j] = seed.time_since_epoch().count() + j + 1;
        meter.measure([&src, &func] {
            Dest dest = src | ops::map(func);
            return dest.begin()->second + 1;
        });
    };

    BENCHMARK_ADVANCED("std::transform() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto func = [](const auto& x) { return std::make_pair(x.first, x.second + (double)1.0); };
        using Dest = WithInnerType_T<TestType, std::invoke_result_t<decltype(func), typename TestType::value_type>>;
        auto seed = std::chrono::system_clock::now();
        TestType src;
        for (int j = 0; j < ContainerSize_V<TestType>; ++j)
            src[seed.time_since_epoch().count() + j] = seed.time_since_epoch().count() + j + 1;
        meter.measure([&src, &func] {
            Dest dest;
            std::transform(src.begin(), src.end(), std::inserter(dest, dest.end()), func);
            return dest.begin()->second + 1;
        });
    };

    BENCHMARK_ADVANCED("cefal::map() - mutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto func = [](auto&& x) { return std::make_pair(std::move(x.first), std::move(x.second) + (double)1.0); };
        using Dest = WithInnerType_T<TestType, std::invoke_result_t<decltype(func), InnerType_T<TestType>>>;
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> src(meter.runs());
        for (auto&& x : src) {
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x[seed.time_since_epoch().count() + j] = seed.time_since_epoch().count() + j + 1;
        }
        meter.measure([&src, &func](int i) {
            Dest dest = std::move(src[i]) | ops::map(func);
            return dest.begin()->second + 1;
        });
    };
}
