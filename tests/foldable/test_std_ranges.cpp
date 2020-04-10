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
    std::vector<std::string> base = {"abc", "de", "f"};
    std::string result = HasOp<Reverse, T...>::value ? "fdeabc" : "abcdef";
    auto get() { return (std::views::all(base) | ... | T()); }
};

template <typename... T>
struct FromSet {
    std::set<std::string> base = {"abc", "de", "f"};
    std::string result = HasOp<Reverse, T...>::value ? "fdeabc" : "abcdef";
    auto get() { return (std::views::all(base) | ... | T()); }
};

template <typename... T>
struct FromSingle {
    std::string result = "abcdef";
    auto get() { return (std::views::single(result) | ... | T()); }
};

TEMPLATE_PRODUCT_TEST_CASE("ops::foldLeft()", "", (FromVector, FromSet, FromSingle),
                           ((Transform), (Filter), (Take), (TakeWhile), (Drop), (DropWhile), (SplitAndJoin), (Reverse),
                            (Transform, Filter, Take, TakeWhile), (Drop, Filter, Take, TakeWhile))) {
    TestType tester;
    std::string result;
    auto folder = [](std::string&& s, auto&& x) {
        auto consumedX = std::forward<decltype(x)>(x);
        return std::move(s) + std::move(consumedX);
    };
    auto left = tester.get();

    SECTION("Lvalue") {
        SECTION("Pipe") { result = left | ops::foldLeft(std::string("result="), folder); }
        SECTION("Curried") { result = ops::foldLeft(std::string("result="), folder)(left); }
    }
    SECTION("Rvalue") {
        SECTION("Pipe") { result = std::move(left) | ops::foldLeft(std::string("result="), folder); }
        SECTION("Curried") { result = ops::foldLeft(std::string("result="), folder)(std::move(left)); }
    }

    CHECK(result == "result=" + tester.result);
}
