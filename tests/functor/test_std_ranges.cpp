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

#include "ranges_helpers.h"

#include "cefal/everything.h"

#include "catch2/catch.hpp"

#include <ranges>
#include <set>
#include <string>
#include <vector>

using namespace cefal;

template <typename... T>
struct FromVector {
    std::vector<std::string> base = {"1", "2", "3"};
    std::vector<int> result = HasOp<Reverse, T...>::value ? std::vector<int>{3, 2, 1} : std::vector<int>{1, 2, 3};
    auto get() { return (std::views::all(base) | ... | T()); }
};

template <typename... T>
struct FromSet {
    std::set<std::string> base = {"1", "2", "3"};
    std::vector<int> result = HasOp<Reverse, T...>::value ? std::vector<int>{3, 2, 1} : std::vector<int>{1, 2, 3};
    auto get() { return (std::views::all(base) | ... | T()); }
};

template <typename... T>
struct FromSingle {
    std::vector<int> result = {42};
    auto get() { return (std::views::single(std::string("42")) | ... | T()); }
};

TEMPLATE_PRODUCT_TEST_CASE("ops::map()", "", (FromVector, FromSet, FromSingle),
                           ((Transform), (Filter), (Take), (TakeWhile), (Drop), (DropWhile), (SplitAndJoin), (Reverse),
                            (Transform, Filter, Take, TakeWhile), (Drop, Filter, Take, TakeWhile))) {
    TestType tester;
    std::vector<int> result;
    auto func = [](auto&& x) -> int {
        auto consumedX = std::forward<decltype(x)>(x);
        return std::stoi(std::move(consumedX));
    };
    auto left = tester.get();

    SECTION("Lvalue") {
        SECTION("Pipe") { result = materialize(left | ops::map(func)); }
        SECTION("Curried") { result = materialize(ops::map(func)(left)); }
    }
    SECTION("Rvalue") {
        SECTION("Pipe") { result = materialize(std::move(left) | ops::map(func)); }
        SECTION("Curried") { result = materialize(ops::map(func)(std::move(left))); }
    }

    CHECK(result == tester.result);
}
