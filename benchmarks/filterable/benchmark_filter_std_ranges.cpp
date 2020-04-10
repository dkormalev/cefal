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

template <typename C>
struct ContainerSize {
    static constexpr size_t value = std::is_same_v<cefal::InnerType_T<C>, int> ? 1'000'000 : 20'000;
};

template <typename T>
constexpr inline size_t ContainerSize_V = ContainerSize<T>::value;

TEMPLATE_PRODUCT_TEST_CASE("cefal::filter()", "", (std::vector), (int)) {
    const std::vector<std::tuple<int, bool, std::string>> descriptors = {{10, false, "10%"},
                                                                         {4, false, "25%"},
                                                                         {2, true, "50%"},
                                                                         {4, true, "75%"},
                                                                         {10, true, "90%"}};

    for (auto&& [limit, keep, percentage] : descriptors) {
        std::function<bool(const InnerType_T<TestType>&)> func;
        if (keep) {
            func = [limit](const InnerType_T<TestType>& x) -> bool { return x % limit; };
        } else {
            func = [limit](const InnerType_T<TestType>& x) -> bool { return !(x % limit); };
        }

        BENCHMARK_ADVANCED("cefal::filter() - " + percentage + " kept - immutable - x" + std::to_string(ContainerSize_V<TestType>))
        (Catch::Benchmark::Chronometer meter) {
            auto seed = std::chrono::system_clock::now();
            TestType src;
            src.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                src.push_back(seed.time_since_epoch().count() + j);
            meter.measure([&src, &func] {
                auto dest = std::views::all(src) | ops::filter(func) | std::views::drop(50) | std::views::take(1);
                return *dest.begin();
            });
        };

        BENCHMARK_ADVANCED("std::views::filter() - " + percentage + " kept - immutable - x"
                           + std::to_string(ContainerSize_V<TestType>))
        (Catch::Benchmark::Chronometer meter) {
            auto seed = std::chrono::system_clock::now();
            TestType src;
            src.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                src.push_back(seed.time_since_epoch().count() + j);
            meter.measure([&src, &func] {
                auto dest = std::views::all(src) | std::views::filter(func) | std::views::drop(50) | std::views::take(1);
                return *dest.begin();
            });
        };
    }
}

TEMPLATE_PRODUCT_TEST_CASE("cefal::filter() - full consume", "", (std::vector), (int)) {
    const std::vector<std::tuple<int, bool, std::string>> descriptors = {{10, false, "10%"},
                                                                         {4, false, "25%"},
                                                                         {2, true, "50%"},
                                                                         {4, true, "75%"},
                                                                         {10, true, "90%"}};

    for (auto&& [limit, keep, percentage] : descriptors) {
        std::function<bool(const InnerType_T<TestType>&)> func;
        if (keep) {
            func = [limit](const InnerType_T<TestType>& x) -> bool { return x % limit; };
        } else {
            func = [limit](const InnerType_T<TestType>& x) -> bool { return !(x % limit); };
        }

        BENCHMARK_ADVANCED("cefal::filter() - " + percentage + " kept - immutable - x" + std::to_string(ContainerSize_V<TestType>))
        (Catch::Benchmark::Chronometer meter) {
            auto seed = std::chrono::system_clock::now();
            TestType src;
            src.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                src.push_back(seed.time_since_epoch().count() + j);
            meter.measure([&src, &func] {
                auto dest = std::views::all(src) | ops::filter(func);
                int result = 0;
                for (int x : dest)
                    result += x;
                return result;
            });
        };

        BENCHMARK_ADVANCED("std::views::filter() - " + percentage + " kept - immutable - x"
                           + std::to_string(ContainerSize_V<TestType>))
        (Catch::Benchmark::Chronometer meter) {
            auto seed = std::chrono::system_clock::now();
            TestType src;
            src.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                src.push_back(seed.time_since_epoch().count() + j);
            meter.measure([&src, &func] {
                auto dest = std::views::all(src) | std::views::filter(func);
                int result = 0;
                for (int x : dest)
                    result += x;
                return result;
            });
        };
    }
}
