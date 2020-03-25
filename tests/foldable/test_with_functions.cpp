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

#include "cefal/instances/foldable/with_functions.h"

#include "catch2/catch.hpp"

#include <string>

using namespace cefal;

struct WithFunctions : public Counter {
    WithFunctions() : Counter() {}
    WithFunctions(int value) : Counter(), value(value) {}
    int value = 0;
    template <typename Result, typename Func>
    Result foldLeft(Result&& init, Func&& f) const& {
        addCustom("lvalue_foldLeft");
        return f(init, value);
    }
    template <typename Result, typename Func>
    Result foldLeft(Result&& init, Func&& f) && {
        addCustom("rvalue_foldLeft");
        return f(init, std::move(value));
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
} // namespace cefal

template <typename T>
struct TemplatedWithFunctions : public Counter {
    TemplatedWithFunctions() : Counter() {}
    TemplatedWithFunctions(int value) : Counter(), value(value) {}
    T value = 0;
    template <typename Result, typename Func>
    Result foldLeft(Result&& init, Func&& f) const& {
        addCustom("lvalue_foldLeft");
        return f(init, value);
    }
    template <typename Result, typename Func>
    Result foldLeft(Result&& init, Func&& f) && {
        addCustom("rvalue_foldLeft");
        return f(init, std::move(value));
    }
};

TEMPLATE_TEST_CASE("ops::foldLeft() - RValue", "", WithFunctions, TemplatedWithFunctions<int>) {
    Counter::reset();
    auto folder = [](double a, int x) -> double { return a + x; };
    SECTION("Pipe") {
        double result = TestType(4) | ops::foldLeft(5.0, folder);
        CHECK(result == Approx(9.0));
    }
    SECTION("Curried") {
        double result = ops::foldLeft(5.0, folder)(TestType(4));
        CHECK(result == Approx(9.0));
    }
    CHECK(Counter::created() == 1);
    CHECK(Counter::copied() == 0);
    CHECK(Counter::moved() == 0);
    CHECK(Counter::customCount() == 1);
    CHECK(Counter::custom("rvalue_foldLeft") == 1);
}

TEMPLATE_TEST_CASE("ops::foldLeft() - LValue", "", WithFunctions, TemplatedWithFunctions<int>) {
    Counter::reset();
    const auto a = TestType(4);
    auto folder = [](double a, int x) -> double { return a + x; };
    SECTION("Pipe") {
        double result = a | ops::foldLeft(5.0, folder);
        CHECK(result == Approx(9.0));
    }
    SECTION("Curried") {
        double result = ops::foldLeft(5.0, folder)(a);
        CHECK(result == Approx(9.0));
    }
    CHECK(Counter::created() == 1);
    CHECK(Counter::copied() == 0);
    CHECK(Counter::moved() == 0);
    CHECK(Counter::customCount() == 1);
    CHECK(Counter::custom("lvalue_foldLeft") == 1);
}
