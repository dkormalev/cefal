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

#include "cefal/everything.h"

#include "catch2/catch.hpp"

#include <string>

using namespace cefal;

struct WithFunctions : public Counter {
    WithFunctions() : Counter() {}
    WithFunctions(int value) : Counter(), value(value) {}
    int value = 0;
    static WithFunctions empty() {
        addCustom("empty");
        return WithFunctions(42);
    }
    WithFunctions append(const WithFunctions& other) const& {
        addCustom("lvalue_append");
        return WithFunctions(value * 10 + other.value);
    }
    WithFunctions append(WithFunctions&& other) && {
        addCustom("rvalue_append");
        value *= 10;
        value += other.value;
        return std::move(*this);
    }
};

TEST_CASE("ops::empty()") {
    Counter::reset();
    WithFunctions result = ops::empty<WithFunctions>();
    CHECK(result.value == 42);
    CHECK(Counter::created() == 1);
    CHECK(Counter::customCount() == 1);
    CHECK(Counter::custom("empty") == 1);
}

TEST_CASE("ops::append() - RValue") {
    Counter::reset();
    SECTION("Pipe") {
        WithFunctions result = WithFunctions(4) | ops::append(WithFunctions(2));
        CHECK(result.value == 42);
    }
    SECTION("Curried") {
        WithFunctions result = ops::append(WithFunctions(2))(WithFunctions(4));
        CHECK(result.value == 42);
    }
    CHECK(Counter::created() == 2);
    CHECK(Counter::copied() == 0);
    CHECK(Counter::moved() == 2);
    CHECK(Counter::customCount() == 1);
    CHECK(Counter::custom("rvalue_append") == 1);
}

TEST_CASE("ops::append() - LValue") {
    Counter::reset();
    const auto a = WithFunctions(4);
    const auto b = WithFunctions(2);
    SECTION("Pipe") {
        WithFunctions result = a | ops::append(b);
        CHECK(result.value == 42);
    }
    SECTION("Curried") {
        WithFunctions result = ops::append(b)(a);
        CHECK(result.value == 42);
    }
    CHECK(Counter::created() == 3);
    CHECK(Counter::copied() == 1);
    CHECK(Counter::customCount() == 1);
    CHECK(Counter::custom("lvalue_append") == 1);
}
