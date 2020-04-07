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
    constexpr static bool hasBase = true;
    std::vector<std::string> base = {"1", "2", "3"};
    std::vector<std::string> result = HasOp<Reverse, T...>::value ? std::vector<std::string>{"3", "1"}
                                                                  : std::vector<std::string>{"1", "3"};
    auto get() { return (std::views::all(base) | ... | T()); }
};

template <typename... T>
struct FromSet {
    constexpr static bool hasBase = true;
    std::set<std::string> base = {"1", "2", "3"};
    std::vector<std::string> result = HasOp<Reverse, T...>::value ? std::vector<std::string>{"3", "1"}
                                                                  : std::vector<std::string>{"1", "3"};
    auto get() { return (std::views::all(base) | ... | T()); }
};

template <typename... T>
struct FromSingle {
    constexpr static bool hasBase = false;
    std::vector<std::string> result = {"13"};
    auto get() { return (std::views::single(std::string("13")) | ... | T()); }
};

// TODO: join_view cases weird compilation error which could be ranges implementation bug
// Check after gcc release
TEMPLATE_PRODUCT_TEST_CASE("ops::filter()", "", (FromVector, FromSet, FromSingle),
                           ((Transform), (Filter), (Take), (TakeWhile), (Drop), (DropWhile), (Reverse), //(SplitAndJoin),

                            (Transform, Own), (Filter, Own), (Take, Own), (TakeWhile, Own), (Drop, Own), (DropWhile, Own),
                            (Reverse, Own), /// (SplitAndJoin, Own),

                            (Own, Transform), (Own, Filter), (Own, Take), (Own, TakeWhile), (Own, Drop), (Own, DropWhile),
                            (Own, Reverse), // (Own, SplitAndJoin),

                            (Transform, Filter, Take, TakeWhile), (Transform, Filter, Take, TakeWhile, Own),
                            (Transform, Filter, Take, Own, TakeWhile), (Transform, Filter, Own, Take, TakeWhile),
                            (Transform, Own, Filter, Take, TakeWhile), (Own, Transform, Filter, Take, TakeWhile),

                            (Drop, Filter, Take, TakeWhile), (Drop, Filter, Take, TakeWhile, Own),
                            (Drop, Filter, Take, Own, TakeWhile), (Drop, Filter, Own, Take, TakeWhile),
                            (Drop, Own, Filter, Take, TakeWhile), (Own, Drop, Filter, Take, TakeWhile))) {
    TestType tester;
    std::vector<std::string> result;
    auto func = [](const std::string& x) -> bool { return std::stoi(x) % 2; };
    auto left = tester.get();

    SECTION("Lvalue") {
        SECTION("Pipe") { result = materialize(left | ops::filter(func)); }
        SECTION("Curried") { result = materialize(ops::filter(func)(left)); }
    }
    SECTION("Rvalue") {
        SECTION("Pipe") { result = materialize(std::move(left) | ops::filter(func)); }
        SECTION("Curried") { result = materialize(ops::filter(func)(std::move(left))); }
    }

    if constexpr (TestType::hasBase) {
        CHECK(tester.base.size() == 3);
        CHECK(!tester.base.begin()->empty());
    }
    CHECK(result == tester.result);
}
