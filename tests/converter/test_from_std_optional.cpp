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
#include "test_helpers.h"

#include "cefal/everything.h"

#include "catch2/catch.hpp"

#include <deque>
#include <list>
#include <set>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>

using namespace cefal;

TEST_CASE("ops::as() - exists") {
    std::vector<CountedValue> result;
    SECTION("Lvalue") {
        const auto left = std::optional<CountedValue>{CountedValue(42)};
        Counter::reset();
        SECTION("Pipe") { result = left | ops::as<std::vector>(); }
        SECTION("Curried") { result = ops::as<std::vector>()(left); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 1);
    }
    SECTION("Rvalue") {
        auto left = std::optional<CountedValue>{CountedValue(42)};
        Counter::reset();
        SECTION("Pipe") { result = std::move(left) | ops::as<std::vector>(); }
        SECTION("Curried") { result = ops::as<std::vector>()(std::move(left)); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 0);
    }
    CHECK(result == std::vector<CountedValue>{CountedValue(42)});
}

TEST_CASE("ops::as() - empty") {
    std::vector<CountedValue> result;
    SECTION("Lvalue") {
        const auto left = std::optional<CountedValue>();
        Counter::reset();
        SECTION("Pipe") { result = left | ops::as<std::vector>(); }
        SECTION("Curried") { result = ops::as<std::vector>()(left); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 0);
    }
    SECTION("Rvalue") {
        auto left = std::optional<CountedValue>();
        Counter::reset();
        SECTION("Pipe") { result = std::move(left) | ops::as<std::vector>(); }
        SECTION("Curried") { result = ops::as<std::vector>()(std::move(left)); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 0);
        CHECK(Counter::moved() == 0);
    }
    CHECK(result.empty());
}
