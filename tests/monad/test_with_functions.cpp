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

#include "counter.h"

#include "cefal/cefal"

#include "cefal/instances/monad/with_functions.h"

#include "catch2/catch.hpp"

#include <string>

using namespace cefal;

struct WithFunctions : public Counter {
    WithFunctions(int value) : Counter(), value(value) {}
    int value = 0;
    static WithFunctions unit(int x) {
        addCustom("unit");
        return WithFunctions(x);
    }
    template <typename Func>
    auto map(Func&& f) const& {
        addCustom("lvalue_map");
        return WithFunctions(f(value));
    }
    template <typename Func>
    auto map(Func&& f) && {
        addCustom("rvalue_map");
        return WithFunctions(f(std::move(value)));
    }
    template <typename Func>
    auto flatMap(Func&& f) const& {
        addCustom("lvalue_map");
        return f(value);
    }
    template <typename Func>
    auto flatMap(Func&& f) && {
        addCustom("rvalue_map");
        return f(std::move(value));
    }
};

struct with_functions : public Counter {
    with_functions(int value) : Counter(), value(value) {}
    int value = 0;
    static with_functions unit(int x) {
        addCustom("unit");
        return with_functions(x);
    }
    template <typename Func>
    auto map(Func&& f) const& {
        addCustom("lvalue_map");
        return with_functions(f(value));
    }
    template <typename Func>
    auto map(Func&& f) && {
        addCustom("rvalue_map");
        return with_functions(f(std::move(value)));
    }
    template <typename Func>
    auto flat_map(Func&& f) const& {
        addCustom("lvalue_map");
        return f(value);
    }
    template <typename Func>
    auto flat_map(Func&& f) && {
        addCustom("rvalue_map");
        return f(std::move(value));
    }
};

namespace cefal {
template <>
struct InnerType<WithFunctions> {
    using type = int;
};
template <typename T>
struct WithInnerType<WithFunctions, T> {
    using type = WithFunctions;
};

template <>
struct InnerType<with_functions> {
    using type = int;
};
template <typename T>
struct WithInnerType<with_functions, T> {
    using type = with_functions;
};
} // namespace cefal

template <typename T>
struct TemplatedWithFunctions : public Counter {
    TemplatedWithFunctions(T value) : Counter(), value(value) {}
    T value = 0;
    static TemplatedWithFunctions unit(T x) {
        addCustom("unit");
        return TemplatedWithFunctions(x);
    }
    template <typename Func>
    auto map(Func&& f) const& {
        addCustom("lvalue_map");
        return TemplatedWithFunctions<std::invoke_result_t<Func, T>>(f(value));
    }
    template <typename Func>
    auto map(Func&& f) && {
        addCustom("rvalue_map");
        return TemplatedWithFunctions<std::invoke_result_t<Func, T>>(f(std::move(value)));
    }
    template <typename Func>
    auto flatMap(Func&& f) const& {
        addCustom("lvalue_map");
        return f(value);
    }
    template <typename Func>
    auto flatMap(Func&& f) && {
        addCustom("rvalue_map");
        return f(std::move(value));
    }
};

TEMPLATE_TEST_CASE("ops::flatMap() - RValue", "", WithFunctions, with_functions, TemplatedWithFunctions<int>) {
    Counter::reset();
    auto func = [](int x) { return WithInnerType_T<TestType, double>(x * 2); };
    SECTION("Pipe") {
        WithInnerType_T<TestType, double> result = TestType(4) | ops::flatMap(func);
        CHECK(result.value == Approx(8.0));
    }
    SECTION("Curried") {
        WithInnerType_T<TestType, double> result = ops::flatMap(func)(TestType(4));
        CHECK(result.value == Approx(8.0));
    }
    CHECK(Counter::created() == 2);
    CHECK(Counter::copied() == 0);
    CHECK(Counter::moved() == 0);
    CHECK(Counter::customCount() == 1);
    CHECK(Counter::custom("rvalue_map") == 1);
}

TEMPLATE_TEST_CASE("ops::flatMap() - LValue", "", WithFunctions, with_functions, TemplatedWithFunctions<int>) {
    Counter::reset();
    const auto a = TestType(4);
    auto func = [](int x) { return WithInnerType_T<TestType, double>(x * 2); };
    SECTION("Pipe") {
        WithInnerType_T<TestType, double> result = a | ops::flatMap(func);
        CHECK(result.value == Approx(8.0));
    }
    SECTION("Curried") {
        WithInnerType_T<TestType, double> result = ops::flatMap(func)(a);
        CHECK(result.value == Approx(8.0));
    }
    CHECK(Counter::created() == 2);
    CHECK(Counter::copied() == 0);
    CHECK(Counter::moved() == 0);
    CHECK(Counter::customCount() == 1);
    CHECK(Counter::custom("lvalue_map") == 1);
}
