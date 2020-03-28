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

template <typename T>
struct TemplatedWithFunctionsWithSingleton : public Counter {
    TemplatedWithFunctionsWithSingleton() : Counter() {}
    TemplatedWithFunctionsWithSingleton(T value) : Counter(), value(value) {}
    T value = T();

    static TemplatedWithFunctionsWithSingleton empty() {
        addCustom("empty");
        return TemplatedWithFunctionsWithSingleton();
    }
    TemplatedWithFunctionsWithSingleton append(const TemplatedWithFunctionsWithSingleton& other) const& {
        addCustom("lvalue_append");
        return TemplatedWithFunctionsWithSingleton(value + other.value);
    }
    TemplatedWithFunctionsWithSingleton append(TemplatedWithFunctionsWithSingleton&& other) && {
        addCustom("rvalue_append");
        value += other.value;
        return std::move(*this);
    }
    TemplatedWithFunctionsWithSingleton
    append(const cefal::helpers::SingletonFrom<TemplatedWithFunctionsWithSingleton<T>>& other) const& {
        addCustom("lvalue_append");
        return TemplatedWithFunctionsWithSingleton(value + other.value);
    }
    TemplatedWithFunctionsWithSingleton append(cefal::helpers::SingletonFrom<TemplatedWithFunctionsWithSingleton<T>>&& other) && {
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

namespace cefal::helpers {
template <typename T>
struct SingletonFrom<TemplatedWithFunctionsWithSingleton<T>> {
    using value_type = T;
    value_type value;
};
} // namespace cefal::helpers

TEMPLATE_PRODUCT_TEST_CASE("ops::filter()", "",
                           (std::vector, std::list, std::deque, std::set, std::unordered_set, std::multiset,
                            std::unordered_multiset),
                           (std::string)) {
    TestType result;
    auto func = [](const std::string& s) { return std::stoi(s) % 2; };
    SECTION("Lvalue") {
        const auto left = TestType{"1", "2", "3"};
        SECTION("Pipe") { result = left | ops::filter(func); }
        SECTION("Curried") { result = ops::filter(func)(left); }
    }
    SECTION("Rvalue") {
        auto left = TestType{"1", "2", "3"};
        SECTION("Pipe") { result = std::move(left) | ops::filter(func); }
        SECTION("Curried") { result = ops::filter(func)(std::move(left)); }
    }

    CHECK(result == TestType{"1", "3"});
}

TEMPLATE_TEST_CASE("ops::filter()", "", TemplatedWithFunctions<std::string>, TemplatedWithFunctionsWithSingleton<std::string>) {
    TestType result;
    auto func = [](const std::string& s) { return true; };
    SECTION("Lvalue") {
        const auto left = TestType("3");
        SECTION("Pipe") { result = left | ops::filter(func); }
        SECTION("Curried") { result = ops::filter(func)(left); }
    }
    SECTION("Rvalue") {
        auto left = TestType("3");
        SECTION("Pipe") { result = std::move(left) | ops::filter(func); }
        SECTION("Curried") { result = ops::filter(func)(std::move(left)); }
    }

    CHECK(result.value == "3");
}
