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

#include <deque>
#include <list>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

using namespace cefal;

template <typename T>
struct TemplatedWithFunctions : public Counter {
    TemplatedWithFunctions() : Counter() {}
    TemplatedWithFunctions(T value) : Counter(), value(value) {}
    T value = T();

    static TemplatedWithFunctions empty() {
        addCustom("empty");
        return TemplatedWithFunctions();
    }
    TemplatedWithFunctions append(const TemplatedWithFunctions& other) const& {
        addCustom("lvalue_append");
        return TemplatedWithFunctions(value + other.value);
    }
    TemplatedWithFunctions append(TemplatedWithFunctions&& other) && {
        addCustom("rvalue_append");
        value += other.value;
        return std::move(*this);
    }

    template <typename Result, typename Func>
    Result foldLeft(Result&& init, Func&& f) const& {
        addCustom("lvalue_foldLeft");
        return f(std::move(init), value);
    }
    template <typename Result, typename Func>
    Result foldLeft(Result&& init, Func&& f) && {
        addCustom("rvalue_foldLeft");
        return f(std::move(init), std::move(value));
    }
};

TEMPLATE_PRODUCT_TEST_CASE("ops::flatMap()", "",
                           (std::vector, std::list, std::deque, std::set, std::unordered_set, std::multiset,
                            std::unordered_multiset),
                           (std::string)) {
    WithInnerType_T<TestType, int> result;
    SECTION("Lvalue") {
        auto func = [](const std::string& s) { return WithInnerType_T<TestType, int>{std::stoi(s)}; };
        const auto left = TestType{"1", "2", "3"};
        SECTION("Pipe") { result = left | ops::flatMap(func); }
        SECTION("Curried") { result = ops::flatMap(func)(left); }
    }
    SECTION("Rvalue") {
        auto func = [](std::string&& s) { return WithInnerType_T<TestType, int>{std::stoi(std::move(s))}; };
        auto left = TestType{"1", "2", "3"};
        SECTION("Pipe") { result = std::move(left) | ops::flatMap(func); }
        SECTION("Curried") { result = ops::flatMap(func)(std::move(left)); }
    }

    CHECK(result == WithInnerType_T<TestType, int>{1, 2, 3});
}

TEMPLATE_PRODUCT_TEST_CASE("ops::flatMap()", "", (std::map, std::unordered_map, std::multimap, std::unordered_multimap),
                           ((std::string, int))) {
    using DestType = WithInnerType_T<TestType, std::pair<int, std::string>>;
    DestType result;
    SECTION("Lvalue") {
        auto func = [](const std::pair<std::string, int>& x) { return DestType{{x.second, x.first}, {x.second * 10, x.first}}; };
        const auto left = TestType{{"abc", 1}, {"de", 2}, {"f", 3}};
        SECTION("Pipe") { result = left | ops::flatMap(func); }
        SECTION("Curried") { result = ops::flatMap(func)(left); }
    }
    SECTION("Rvalue") {
        auto func = [](std::pair<std::string, int>&& x) { return DestType{{x.second, x.first}, {x.second * 10, x.first}}; };
        auto left = TestType{{"abc", 1}, {"de", 2}, {"f", 3}};
        SECTION("Pipe") { result = std::move(left) | ops::flatMap(func); }
        SECTION("Curried") { result = ops::flatMap(func)(std::move(left)); }
    }

    CHECK(result == DestType{{1, "abc"}, {2, "de"}, {3, "f"}, {10, "abc"}, {20, "de"}, {30, "f"}});
}

TEST_CASE("ops::flatMap() - TemplatedWithFunctions<std::string>") {
    TemplatedWithFunctions<int> result;
    SECTION("Lvalue") {
        auto func = [](const std::string& s) { return TemplatedWithFunctions<int>(std::stoi(s)); };
        const auto left = TemplatedWithFunctions<std::string>("3");
        SECTION("Pipe") { result = left | ops::flatMap(func); }
        SECTION("Curried") { result = ops::flatMap(func)(left); }
    }
    SECTION("Rvalue") {
        auto func = [](std::string&& s) { return TemplatedWithFunctions<int>(std::stoi(std::move(s))); };
        auto left = TemplatedWithFunctions<std::string>("3");
        SECTION("Pipe") { result = std::move(left) | ops::flatMap(func); }
        SECTION("Curried") { result = ops::flatMap(func)(std::move(left)); }
    }

    CHECK(result.value == 3);
}
