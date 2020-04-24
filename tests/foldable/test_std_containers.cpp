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

#include "test_helpers.h"

#include "cefal/everything.h"

#include "catch2/catch.hpp"

#include <deque>
#include <list>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

using namespace cefal;

TEMPLATE_PRODUCT_TEST_CASE("ops::foldLeft()", "",
                           (std::vector, std::list, std::deque, std::set, std::unordered_set, std::multiset,
                            std::unordered_multiset),
                           (int)) {
    std::string result;
    std::string expected = "result=";
    auto folder = [](std::string&& s, int x) {
        s += std::to_string(x);
        return std::move(s);
    };
    for (auto&& x : TestType{1, 2, 3})
        expected += std::to_string(x);
    SECTION("Lvalue") {
        const auto left = TestType{1, 2, 3};
        SECTION("Pipe") { result = left | ops::foldLeft(std::string("result="), folder); }
        SECTION("Curried") { result = ops::foldLeft(std::string("result="), folder)(left); }
    }
    SECTION("Rvalue") {
        auto left = TestType{1, 2, 3};
        SECTION("Pipe") { result = std::move(left) | ops::foldLeft(std::string("result="), folder); }
        SECTION("Curried") { result = ops::foldLeft(std::string("result="), folder)(std::move(left)); }
    }

    CHECK(result == expected);
}

TEMPLATE_PRODUCT_TEST_CASE("ops::foldLeft()", "",
                           (std::vector, std::list, std::deque, std::set, std::unordered_set, std::multiset,
                            std::unordered_multiset),
                           (std::string)) {
    std::string result;
    std::string expected = "result=";
    for (auto&& x : TestType{"abc", "de", "f"})
        expected += x;
    SECTION("Lvalue") {
        auto folder = [](std::string&& s, const std::string& x) { return std::move(s) + x; };
        const auto left = TestType{"abc", "de", "f"};
        SECTION("Pipe") { result = left | ops::foldLeft(std::string("result="), folder); }
        SECTION("Curried") { result = ops::foldLeft(std::string("result="), folder)(left); }
    }
    SECTION("Rvalue") {
        auto folder = [](std::string&& s, std::string&& x) { return std::move(s) + std::move(x); };
        auto left = TestType{"abc", "de", "f"};
        SECTION("Pipe") { result = std::move(left) | ops::foldLeft(std::string("result="), folder); }
        SECTION("Curried") { result = ops::foldLeft(std::string("result="), folder)(std::move(left)); }
    }

    CHECK(result == expected);
}

TEMPLATE_PRODUCT_TEST_CASE("ops::foldLeft()", "", (std::map, std::unordered_map, std::multimap, std::unordered_multimap),
                           ((std::string, int))) {
    std::string result;
    std::string expected = "result=";
    for (auto&& x : TestType{{"abc", 1}, {"de", 2}, {"f", 3}})
        expected += x.first + std::string(' ', x.second);
    SECTION("Lvalue") {
        auto folder = [](std::string&& s, const std::pair<std::string, int>& x) {
            return std::move(s) + x.first + std::string(' ', x.second);
        };
        const auto left = TestType{{"abc", 1}, {"de", 2}, {"f", 3}};
        SECTION("Pipe") { result = left | ops::foldLeft(std::string("result="), folder); }
        SECTION("Curried") { result = ops::foldLeft(std::string("result="), folder)(left); }
    }
    SECTION("Rvalue") {
        auto folder = [](std::string&& s, std::pair<std::string, int>&& x) {
            return std::move(s) + std::move(x.first) + std::string(' ', x.second);
        };
        auto left = TestType{{"abc", 1}, {"de", 2}, {"f", 3}};
        SECTION("Pipe") { result = std::move(left) | ops::foldLeft(std::string("result="), folder); }
        SECTION("Curried") { result = ops::foldLeft(std::string("result="), folder)(std::move(left)); }
    }

    CHECK(result == expected);
}
