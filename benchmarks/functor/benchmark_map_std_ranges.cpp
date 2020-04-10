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

#include <iostream>
#include <list>
#include <ranges>
#include <string>
#include <vector>

using namespace cefal;

// It has its own Expensive due to heavy optimization that could be done for ranges operations by compiler
// Regular expensive structure kept as is for purposes of containers benchmarking
template <typename T>
struct Expensive {
    Expensive(T x) : value(std::move(x)) { init(); }
    Expensive(T x, int* payload) : value(std::move(x)), payload(payload) {}
    Expensive(const Expensive& other) {
        init();
        value = other.value;
    }
    Expensive(Expensive&& other) {
        payload = other.payload;
        value = std::move(other.value);
        other.payload = nullptr;
    }
    Expensive& operator=(const Expensive& other) {
        init();
        value = other.value;
        return *this;
    }
    Expensive& operator=(Expensive&& other) {
        std::swap(payload, other.payload);
        value = std::move(other.value);
        other.payload = nullptr;
        return *this;
    }
    ~Expensive() { delete[] payload; }

    operator T() const { return value + payload[std::chrono::system_clock::now().time_since_epoch().count() & 0xFF]; }

    template <typename U>
    bool operator%(U&& other) const {
        return value % other;
    }

    template <typename U>
    Expensive<std::remove_cvref_t<U>> operator+(U&& other) const& {
        return Expensive<std::remove_cvref_t<U>>(value + other);
    }

    template <typename U>
    Expensive<std::remove_cvref_t<U>> operator+(U&& other) && {
        auto oldPayload = payload;
        payload = 0;
        return Expensive<std::remove_cvref_t<U>>(value + other, oldPayload);
    }

    auto operator<=>(const Expensive<T>& other) const { return value <=> other.value; }
    bool operator==(const Expensive<T>& other) const { return std::abs(value - other.value) < 0.0001; }

    void init() {
        delete[] payload;
        payload = new int[1024];
        auto seed = std::chrono::system_clock::now().time_since_epoch().count();
        for (int i = 0; i < 1024; ++i)
            payload[i] = seed + i;
    }

    T value = T();
    int* payload = nullptr;
};

template <typename C>
struct ContainerSize {
    static constexpr size_t value = std::is_same_v<cefal::InnerType_T<C>, int> ? 1'000'000 : 20'000;
};

template <typename T>
constexpr inline size_t ContainerSize_V = ContainerSize<T>::value;

TEMPLATE_PRODUCT_TEST_CASE("cefal::map() to same type", "", (std::vector), (int, Expensive<int>)) {
    auto seed = std::chrono::system_clock::now();
    auto copyFunc = [seed](InnerType_T<TestType>& x) { return x + (int)seed.time_since_epoch().count(); };
    auto moveFunc = [seed](InnerType_T<TestType>& x) { return std::move(x) + (int)seed.time_since_epoch().count(); };

    BENCHMARK_ADVANCED("cefal::map() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> rawSrc(meter.runs());
        std::vector<decltype(std::views::all(*rawSrc.begin()))> src;
        for (auto&& x : rawSrc) {
            x.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.push_back(seed.time_since_epoch().count() + j);
            src.push_back(std::views::all(x));
        }
        meter.measure([&src, &copyFunc](int i) {
            auto dest = src[i] | ops::map(copyFunc) | std::views::drop(50) | std::views::take(1);
            return *dest.begin();
        });
    };

    BENCHMARK_ADVANCED("std::views::transform() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> rawSrc(meter.runs());
        std::vector<decltype(std::views::all(*rawSrc.begin()))> src;
        for (auto&& x : rawSrc) {
            x.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.push_back(seed.time_since_epoch().count() + j);
            src.push_back(std::views::all(x));
        }
        meter.measure([&src, &copyFunc](int i) {
            auto dest = src[i] | std::views::transform(copyFunc) | std::views::drop(50) | std::views::take(1);
            return *dest.begin();
        });
    };

    BENCHMARK_ADVANCED("cefal::map() - mutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> rawSrc(meter.runs());
        std::vector<decltype(std::views::all(*rawSrc.begin()))> src;
        for (auto&& x : rawSrc) {
            x.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.push_back(seed.time_since_epoch().count() + j);
            src.push_back(std::views::all(x));
        }
        meter.measure([&src, &moveFunc](int i) {
            auto dest = src[i] | ops::map(moveFunc) | std::views::drop(50) | std::views::take(1);
            return *dest.begin();
        });
    };

    BENCHMARK_ADVANCED("std::views::transform() - mutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> rawSrc(meter.runs());
        std::vector<decltype(std::views::all(*rawSrc.begin()))> src;
        for (auto&& x : rawSrc) {
            x.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.push_back(seed.time_since_epoch().count() + j);
            src.push_back(std::views::all(x));
        }
        meter.measure([&src, &moveFunc](int i) {
            auto dest = src[i] | std::views::transform(moveFunc) | std::views::drop(50) | std::views::take(1);
            return *dest.begin();
        });
    };
}

TEMPLATE_PRODUCT_TEST_CASE("cefal::map() to different type", "", (std::vector), (int, Expensive<int>)) {
    auto seed = std::chrono::system_clock::now();
    auto copyFunc = [seed](InnerType_T<TestType>& x) { return x + (double)seed.time_since_epoch().count(); };
    auto moveFunc = [seed](InnerType_T<TestType>& x) { return std::move(x) + (double)seed.time_since_epoch().count(); };
    using Dest = WithInnerType_T<TestType, std::invoke_result_t<decltype(copyFunc), InnerType_T<TestType>&>>;

    BENCHMARK_ADVANCED("cefal::map() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> rawSrc(meter.runs());
        std::vector<decltype(std::views::all(*rawSrc.begin()))> src;
        for (auto&& x : rawSrc) {
            x.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.push_back(seed.time_since_epoch().count() + j);
            src.push_back(std::views::all(x));
        }
        meter.measure([&src, &copyFunc](int i) {
            auto dest = src[i] | ops::map(copyFunc) | std::views::drop(50) | std::views::take(1);
            return *dest.begin();
        });
    };

    BENCHMARK_ADVANCED("std::views::transform() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> rawSrc(meter.runs());
        std::vector<decltype(std::views::all(*rawSrc.begin()))> src;
        for (auto&& x : rawSrc) {
            x.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.push_back(seed.time_since_epoch().count() + j);
            src.push_back(std::views::all(x));
        }
        meter.measure([&src, &copyFunc](int i) {
            auto dest = src[i] | std::views::transform(copyFunc) | std::views::drop(50) | std::views::take(1);
            return *dest.begin();
        });
    };

    BENCHMARK_ADVANCED("cefal::map() - mutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> rawSrc(meter.runs());
        std::vector<decltype(std::views::all(*rawSrc.begin()))> src;
        for (auto&& x : rawSrc) {
            x.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.push_back(seed.time_since_epoch().count() + j);
            src.push_back(std::views::all(x));
        }
        meter.measure([&src, &moveFunc](int i) {
            auto dest = src[i] | ops::map(moveFunc) | std::views::drop(50) | std::views::take(1);
            return *dest.begin();
        });
    };

    BENCHMARK_ADVANCED("std::views::transform() - mutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> rawSrc(meter.runs());
        std::vector<decltype(std::views::all(*rawSrc.begin()))> src;
        for (auto&& x : rawSrc) {
            x.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.push_back(seed.time_since_epoch().count() + j);
            src.push_back(std::views::all(x));
        }
        meter.measure([&src, &moveFunc](int i) {
            auto dest = src[i] | std::views::transform(moveFunc) | std::views::drop(50) | std::views::take(1);
            return *dest.begin();
        });
    };
}

TEMPLATE_PRODUCT_TEST_CASE("cefal::map() to same type - full consume", "", (std::vector), (int, Expensive<int>)) {
    auto seed = std::chrono::system_clock::now();
    auto copyFunc = [seed](InnerType_T<TestType>& x) { return x + (int)seed.time_since_epoch().count(); };
    auto moveFunc = [seed](InnerType_T<TestType>& x) { return std::move(x) + (int)seed.time_since_epoch().count(); };

    BENCHMARK_ADVANCED("cefal::map() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> rawSrc(meter.runs());
        std::vector<decltype(std::views::all(*rawSrc.begin()))> src;
        for (auto&& x : rawSrc) {
            x.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.push_back(seed.time_since_epoch().count() + j);
            src.push_back(std::views::all(x));
        }
        meter.measure([&src, &copyFunc](int i) {
            auto dest = src[i] | ops::map(copyFunc);
            int result = 0;
            for (int x : dest)
                result += x;
            return result;
        });
    };

    BENCHMARK_ADVANCED("std::views::transform() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> rawSrc(meter.runs());
        std::vector<decltype(std::views::all(*rawSrc.begin()))> src;
        for (auto&& x : rawSrc) {
            x.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.push_back(seed.time_since_epoch().count() + j);
            src.push_back(std::views::all(x));
        }
        meter.measure([&src, &copyFunc](int i) {
            auto dest = src[i] | std::views::transform(copyFunc);
            int result = 0;
            for (int x : dest)
                result += x;
            return result;
        });
    };

    BENCHMARK_ADVANCED("cefal::map() - mutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> rawSrc(meter.runs());
        std::vector<decltype(std::views::all(*rawSrc.begin()))> src;
        for (auto&& x : rawSrc) {
            x.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.push_back(seed.time_since_epoch().count() + j);
            src.push_back(std::views::all(x));
        }
        meter.measure([&src, &moveFunc](int i) {
            auto dest = src[i] | ops::map(moveFunc);
            int result = 0;
            for (int x : dest)
                result += x;
            return result;
        });
    };

    BENCHMARK_ADVANCED("std::views::transform() - mutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> rawSrc(meter.runs());
        std::vector<decltype(std::views::all(*rawSrc.begin()))> src;
        for (auto&& x : rawSrc) {
            x.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.push_back(seed.time_since_epoch().count() + j);
            src.push_back(std::views::all(x));
        }
        meter.measure([&src, &moveFunc](int i) {
            auto dest = src[i] | std::views::transform(moveFunc);
            int result = 0;
            for (int x : dest)
                result += x;
            return result;
        });
    };
}

TEMPLATE_PRODUCT_TEST_CASE("cefal::map() to different type - full consume", "", (std::vector), (int, Expensive<int>)) {
    auto seed = std::chrono::system_clock::now();
    auto copyFunc = [seed](InnerType_T<TestType>& x) { return x + (double)seed.time_since_epoch().count(); };
    auto moveFunc = [seed](InnerType_T<TestType>& x) { return std::move(x) + (double)seed.time_since_epoch().count(); };
    using Dest = WithInnerType_T<TestType, std::invoke_result_t<decltype(copyFunc), InnerType_T<TestType>&>>;

    BENCHMARK_ADVANCED("cefal::map() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> rawSrc(meter.runs());
        std::vector<decltype(std::views::all(*rawSrc.begin()))> src;
        for (auto&& x : rawSrc) {
            x.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.push_back(seed.time_since_epoch().count() + j);
            src.push_back(std::views::all(x));
        }
        meter.measure([&src, &copyFunc](int i) {
            auto dest = src[i] | ops::map(copyFunc);
            double result = 0;
            for (const auto& x : dest)
                result += (double)x;
            return result;
        });
    };

    BENCHMARK_ADVANCED("std::views::transform() - immutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> rawSrc(meter.runs());
        std::vector<decltype(std::views::all(*rawSrc.begin()))> src;
        for (auto&& x : rawSrc) {
            x.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.push_back(seed.time_since_epoch().count() + j);
            src.push_back(std::views::all(x));
        }
        meter.measure([&src, &copyFunc](int i) {
            auto dest = src[i] | std::views::transform(copyFunc);
            double result = 0;
            for (double x : dest)
                result += x;
            return result;
        });
    };

    BENCHMARK_ADVANCED("cefal::map() - mutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> rawSrc(meter.runs());
        std::vector<decltype(std::views::all(*rawSrc.begin()))> src;
        for (auto&& x : rawSrc) {
            x.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.push_back(seed.time_since_epoch().count() + j);
            src.push_back(std::views::all(x));
        }
        meter.measure([&src, &moveFunc](int i) {
            auto dest = src[i] | ops::map(moveFunc);
            double result = 0;
            for (const auto& x : dest)
                result += (double)x;
            return result;
        });
    };

    BENCHMARK_ADVANCED("std::views::transform() - mutable - x" + std::to_string(ContainerSize_V<TestType>))
    (Catch::Benchmark::Chronometer meter) {
        auto seed = std::chrono::system_clock::now();
        std::vector<TestType> rawSrc(meter.runs());
        std::vector<decltype(std::views::all(*rawSrc.begin()))> src;
        for (auto&& x : rawSrc) {
            x.reserve(ContainerSize_V<TestType>);
            for (int j = 0; j < ContainerSize_V<TestType>; ++j)
                x.push_back(seed.time_since_epoch().count() + j);
            src.push_back(std::views::all(x));
        }
        meter.measure([&src, &moveFunc](int i) {
            auto dest = src[i] | std::views::transform(moveFunc);
            double result = 0;
            for (double x : dest)
                result += x;
            return result;
        });
    };
}
