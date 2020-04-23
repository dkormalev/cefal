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

template <typename Src, typename Dest>
struct PrepareResult {
    static constexpr Dest prepare(Dest dest) { return std::move(dest); }
};
template <typename T, std::ranges::random_access_range Dest>
struct PrepareResult<std::unordered_set<T>, Dest> {
    static constexpr Dest prepare(Dest dest) {
        std::ranges::sort(dest);
        return std::move(dest);
    }
};
template <typename T, std::ranges::random_access_range Dest>
struct PrepareResult<std::unordered_multiset<T>, Dest> {
    static constexpr Dest prepare(Dest dest) {
        std::ranges::sort(dest);
        return std::move(dest);
    }
};
template <typename K, typename V, std::ranges::random_access_range Dest>
struct PrepareResult<std::unordered_map<K, V>, Dest> {
    static constexpr Dest prepare(Dest dest) {
        std::ranges::sort(dest);
        return std::move(dest);
    }
};
template <typename K, typename V, std::ranges::random_access_range Dest>
struct PrepareResult<std::unordered_multimap<K, V>, Dest> {
    static constexpr Dest prepare(Dest dest) {
        std::ranges::sort(dest);
        return std::move(dest);
    }
};

template <typename T, typename U>
struct PrepareResult<std::unordered_set<T>, std::list<U>> {
    static constexpr std::list<U> prepare(std::list<U> dest) {
        std::vector<U> tmp(dest.begin(), dest.end());
        std::ranges::sort(tmp);
        return std::list<U>(tmp.begin(), tmp.end());
    }
};
template <typename T, typename U>
struct PrepareResult<std::unordered_multiset<T>, std::list<U>> {
    static constexpr std::list<U> prepare(std::list<U> dest) {
        std::vector<U> tmp(dest.begin(), dest.end());
        std::ranges::sort(tmp);
        return std::list<U>(tmp.begin(), tmp.end());
    }
};
template <typename K, typename V, typename U>
struct PrepareResult<std::unordered_map<K, V>, std::list<U>> {
    static constexpr std::list<U> prepare(std::list<U> dest) {
        std::vector<U> tmp(dest.begin(), dest.end());
        std::ranges::sort(tmp);
        return std::list<U>(tmp.begin(), tmp.end());
    }
};
template <typename K, typename V, typename U>
struct PrepareResult<std::unordered_multimap<K, V>, std::list<U>> {
    static constexpr std::list<U> prepare(std::list<U> dest) {
        std::vector<U> tmp(dest.begin(), dest.end());
        std::ranges::sort(tmp);
        return std::list<U>(tmp.begin(), tmp.end());
    }
};

template <typename...>
struct TypesProduct;

template <typename... Ts, typename U, typename... Us>
struct TypesProduct<std::tuple<Ts...>, std::tuple<U, Us...>> {
    using type = decltype(
        std::tuple_cat(std::tuple<std::tuple<Ts, U>...>(), typename TypesProduct<std::tuple<Ts...>, std::tuple<Us...>>::type()));
};

template <typename... Ts>
struct TypesProduct<std::tuple<Ts...>, std::tuple<>> {
    using type = std::tuple<>;
};

using SingleSocketContainers =
    std::tuple<std::vector<CountedValue>, std::list<CountedValue>, std::deque<CountedValue>, std::set<CountedValue>,
               std::unordered_set<CountedValue>, std::multiset<CountedValue>, std::unordered_multiset<CountedValue>>;
using DoubleSocketContainers = std::tuple<std::map<int, CountedValue>, std::unordered_map<int, CountedValue>,
                                          std::multimap<int, CountedValue>, std::unordered_multimap<int, CountedValue>>;
using HeavyDoubleSocketContainers =
    std::tuple<std::map<CountedValue, CountedValue>, std::unordered_map<CountedValue, CountedValue>,
               std::multimap<CountedValue, CountedValue>, std::unordered_multimap<CountedValue, CountedValue>>;

using SingleToSingle = typename TypesProduct<SingleSocketContainers, SingleSocketContainers>::type;
using SingleToDouble = typename TypesProduct<SingleSocketContainers, DoubleSocketContainers>::type;
using DoubleToSingle = typename TypesProduct<DoubleSocketContainers, SingleSocketContainers>::type;
using DoubleToDouble = typename TypesProduct<DoubleSocketContainers, DoubleSocketContainers>::type;
using HeavyDoubleToSingle = typename TypesProduct<HeavyDoubleSocketContainers, SingleSocketContainers>::type;
using HeavyDoubleToDouble = typename TypesProduct<HeavyDoubleSocketContainers, HeavyDoubleSocketContainers>::type;

using namespace cefal;

TEMPLATE_LIST_TEST_CASE("ops::as()", "", SingleToSingle) {
    using Src = std::tuple_element_t<0, TestType>;
    using Dest = std::tuple_element_t<1, TestType>;
    Dest result;
    SECTION("Lvalue") {
        const auto left = Src{CountedValue(1), CountedValue(2), CountedValue(3)};
        Counter::reset();
        SECTION("Pipe") { result = left | ops::as<Dest>(); }
        SECTION("Curried") { result = ops::as<Dest>()(left); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 3);
    }
    SECTION("Rvalue") {
        auto left = Src{CountedValue(1), CountedValue(2), CountedValue(3)};
        Counter::reset();
        SECTION("Pipe") { result = std::move(left) | ops::as<Dest>(); }
        SECTION("Curried") { result = ops::as<Dest>()(std::move(left)); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 0);
        CHECK(Counter::moved() <= 3);
    }
    result = PrepareResult<Src, Dest>::prepare(std::move(result));
    CHECK(result == Dest{CountedValue(1), CountedValue(2), CountedValue(3)});
}

TEMPLATE_LIST_TEST_CASE("ops::as()", "", DoubleToDouble) {
    using Src = std::tuple_element_t<0, TestType>;
    using Dest = std::tuple_element_t<1, TestType>;
    Dest result;
    SECTION("Lvalue") {
        const auto left = Src{{1, CountedValue(1)}, {2, CountedValue(2)}, {3, CountedValue(3)}};
        Counter::reset();
        SECTION("Pipe") { result = left | ops::as<Dest>(); }
        SECTION("Curried") { result = ops::as<Dest>()(left); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 3);
    }
    SECTION("Rvalue") {
        auto left = Src{{1, CountedValue(1)}, {2, CountedValue(2)}, {3, CountedValue(3)}};
        Counter::reset();
        SECTION("Pipe") { result = std::move(left) | ops::as<Dest>(); }
        SECTION("Curried") { result = ops::as<Dest>()(std::move(left)); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 0);
        CHECK(Counter::moved() <= 3);
    }
    CHECK(result == Dest{{1, CountedValue(1)}, {2, CountedValue(2)}, {3, CountedValue(3)}});
}

TEMPLATE_LIST_TEST_CASE("ops::as()", "", SingleToDouble) {
    using Src = std::tuple_element_t<0, TestType>;
    using Dest = std::tuple_element_t<1, TestType>;
    Dest result;
    SECTION("Lvalue") {
        const auto left = Src{CountedValue(1), CountedValue(2), CountedValue(3)}
                          | ops::map([](CountedValue&& x) { return std::make_tuple(x.value, x); });
        Counter::reset();
        SECTION("Pipe") { result = left | ops::as<Dest>(); }
        SECTION("Curried") { result = ops::as<Dest>()(left); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 3);
    }
    SECTION("Rvalue") {
        auto left = Src{CountedValue(1), CountedValue(2), CountedValue(3)}
                    | ops::map([](CountedValue&& x) { return std::make_tuple(x.value, x); });
        Counter::reset();
        SECTION("Pipe") { result = std::move(left) | ops::as<Dest>(); }
        SECTION("Curried") { result = ops::as<Dest>()(std::move(left)); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 0);
        CHECK(Counter::moved() <= 3);
    }
    CHECK(result == Dest{{1, CountedValue(1)}, {2, CountedValue(2)}, {3, CountedValue(3)}});
}

TEMPLATE_LIST_TEST_CASE("ops::as()", "", DoubleToSingle) {
    using Src = std::tuple_element_t<0, TestType>;
    using Dest = WithInnerType_T<std::tuple_element_t<1, TestType>, std::tuple<int, CountedValue>>;
    Dest result;
    SECTION("Lvalue") {
        const auto left = Src{{1, CountedValue(1)}, {2, CountedValue(2)}, {3, CountedValue(3)}};
        Counter::reset();
        SECTION("Pipe") { result = left | ops::as<Dest>(); }
        SECTION("Curried") { result = ops::as<Dest>()(left); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 3);
    }
    SECTION("Rvalue") {
        auto left = Src{{1, CountedValue(1)}, {2, CountedValue(2)}, {3, CountedValue(3)}};
        Counter::reset();
        SECTION("Pipe") { result = std::move(left) | ops::as<Dest>(); }
        SECTION("Curried") { result = ops::as<Dest>()(std::move(left)); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 0);
        CHECK(Counter::moved() <= 9);
    }
    result = PrepareResult<Src, Dest>::prepare(std::move(result));
    CHECK(result == Dest{{1, CountedValue(1)}, {2, CountedValue(2)}, {3, CountedValue(3)}});
}

TEMPLATE_LIST_TEST_CASE("ops::as()", "", HeavyDoubleToDouble) {
    using Src = std::tuple_element_t<0, TestType>;
    using Dest = std::tuple_element_t<1, TestType>;
    Dest result;
    SECTION("Lvalue") {
        const auto left = Src{{CountedValue(11), CountedValue(1)},
                              {CountedValue(12), CountedValue(2)},
                              {CountedValue(13), CountedValue(3)}};
        Counter::reset();
        SECTION("Pipe") { result = left | ops::as<Dest>(); }
        SECTION("Curried") { result = ops::as<Dest>()(left); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 6);
    }
    SECTION("Rvalue") {
        auto left = Src{{CountedValue(11), CountedValue(1)},
                        {CountedValue(12), CountedValue(2)},
                        {CountedValue(13), CountedValue(3)}};
        Counter::reset();
        SECTION("Pipe") { result = std::move(left) | ops::as<Dest>(); }
        SECTION("Curried") { result = ops::as<Dest>()(std::move(left)); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 0);
        CHECK(Counter::moved() <= 6);
    }
    CHECK(result
          == Dest{{CountedValue(11), CountedValue(1)}, {CountedValue(12), CountedValue(2)}, {CountedValue(13), CountedValue(3)}});
}

TEMPLATE_LIST_TEST_CASE("ops::as()", "", HeavyDoubleToSingle) {
    using Src = std::tuple_element_t<0, TestType>;
    using Dest = WithInnerType_T<std::tuple_element_t<1, TestType>, std::tuple<CountedValue, CountedValue>>;
    Dest result;
    SECTION("Lvalue") {
        const auto left = Src{{CountedValue(11), CountedValue(1)},
                              {CountedValue(12), CountedValue(2)},
                              {CountedValue(13), CountedValue(3)}};
        Counter::reset();
        SECTION("Pipe") { result = left | ops::as<Dest>(); }
        SECTION("Curried") { result = ops::as<Dest>()(left); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 6);
    }
    SECTION("Rvalue") {
        auto left = Src{{CountedValue(11), CountedValue(1)},
                        {CountedValue(12), CountedValue(2)},
                        {CountedValue(13), CountedValue(3)}};
        Counter::reset();
        SECTION("Pipe") { result = std::move(left) | ops::as<Dest>(); }
        SECTION("Curried") { result = ops::as<Dest>()(std::move(left)); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 0);
        CHECK(Counter::moved() <= 18);
    }
    result = PrepareResult<Src, Dest>::prepare(std::move(result));
    CHECK(result
          == Dest{{CountedValue(11), CountedValue(1)}, {CountedValue(12), CountedValue(2)}, {CountedValue(13), CountedValue(3)}});
}

TEMPLATE_LIST_TEST_CASE("ops::as() - from view", "", SingleSocketContainers) {
    TestType result;
    SECTION("Lvalue") {
        const std::vector<CountedValue> src{CountedValue(1), CountedValue(2), CountedValue(3)};
        const auto left = std::views::all(src);
        Counter::reset();
        SECTION("Pipe") { result = left | ops::as<TestType>(); }
        SECTION("Curried") { result = ops::as<TestType>()(left); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 3);
    }
    SECTION("Rvalue") {
        const std::vector<CountedValue> src{CountedValue(1), CountedValue(2), CountedValue(3)};
        auto left = std::views::all(src);
        Counter::reset();
        SECTION("Pipe") { result = std::move(left) | ops::as<TestType>(); }
        SECTION("Curried") { result = ops::as<TestType>()(std::move(left)); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 3);
    }
    CHECK(result == TestType{CountedValue(1), CountedValue(2), CountedValue(3)});
}

TEMPLATE_LIST_TEST_CASE("ops::as() - from view", "", HeavyDoubleSocketContainers) {
    TestType result;
    SECTION("Lvalue") {
        const std::vector<std::tuple<CountedValue, CountedValue>> src{{CountedValue(11), CountedValue(1)},
                                                                      {CountedValue(12), CountedValue(2)},
                                                                      {CountedValue(13), CountedValue(3)}};
        const auto left = std::views::all(src);
        Counter::reset();
        SECTION("Pipe") { result = left | ops::as<TestType>(); }
        SECTION("Curried") { result = ops::as<TestType>()(left); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 6);
    }
    SECTION("Rvalue") {
        const std::vector<std::tuple<CountedValue, CountedValue>> src{{CountedValue(11), CountedValue(1)},
                                                                      {CountedValue(12), CountedValue(2)},
                                                                      {CountedValue(13), CountedValue(3)}};
        const auto left = std::views::all(src);
        Counter::reset();
        SECTION("Pipe") { result = std::move(left) | ops::as<TestType>(); }
        SECTION("Curried") { result = ops::as<TestType>()(std::move(left)); }
        CHECK(Counter::created() == 0);
        CHECK(Counter::copied() == 6);
    }
    CHECK(result
          == TestType{{CountedValue(11), CountedValue(1)}, {CountedValue(12), CountedValue(2)}, {CountedValue(13), CountedValue(3)}});
}
