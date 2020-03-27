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

#include "cefal/cefal"

#include "cefal/instances/monad/std_optional.h"

#include "catch2/catch.hpp"

#include <optional>

using namespace cefal;

TEST_CASE("ops::flatMap() - exists - returns value") {
    std::optional<int> result;
    SECTION("Lvalue") {
        auto func = [](const std::string& x) -> std::optional<int> { return std::stoi(x); };
        const auto left = std::optional<std::string>("42");
        SECTION("Pipe") { result = left | ops::flatMap(func); }
        SECTION("Curried") { result = ops::flatMap(func)(left); }
    }
    SECTION("Rvalue") {
        auto func = [](std::string&& x) -> std::optional<int> { return std::stoi(x); };
        auto left = std::optional<std::string>("42");
        SECTION("Pipe") { result = std::move(left) | ops::flatMap(func); }
        SECTION("Curried") { result = ops::flatMap(func)(std::move(left)); }
    }

    REQUIRE(result);
    CHECK(*result == 42);
}

TEST_CASE("ops::flatMap() - exists - returns nullopt") {
    std::optional<int> result;
    SECTION("Lvalue") {
        auto func = [](const std::string& x) -> std::optional<int> { return std::nullopt; };
        const auto left = std::optional<std::string>("42");
        SECTION("Pipe") { result = left | ops::flatMap(func); }
        SECTION("Curried") { result = ops::flatMap(func)(left); }
    }
    SECTION("Rvalue") {
        auto func = [](std::string&& x) -> std::optional<int> { return std::nullopt; };
        auto left = std::optional<std::string>("42");
        SECTION("Pipe") { result = std::move(left) | ops::flatMap(func); }
        SECTION("Curried") { result = ops::flatMap(func)(std::move(left)); }
    }

    CHECK(!result);
}

TEST_CASE("ops::flatMap() - nullopt") {
    std::optional<int> result;
    bool called = false;
    auto func = [&called](const std::string& x) -> std::optional<int> {
        called = true;
        return std::stoi(x);
    };
    SECTION("Lvalue") {
        const std::optional<std::string> left = std::nullopt;
        SECTION("Pipe") { result = left | ops::flatMap(func); }
        SECTION("Curried") { result = ops::flatMap(func)(left); }
    }
    SECTION("Rvalue") {
        std::optional<std::string> left = std::nullopt;
        SECTION("Pipe") { result = std::move(left) | ops::flatMap(func); }
        SECTION("Curried") { result = ops::flatMap(func)(std::move(left)); }
    }

    CHECK(!result);
    CHECK(!called);
}
