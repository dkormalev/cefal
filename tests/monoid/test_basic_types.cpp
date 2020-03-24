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

#include "cefal/cefal"

#include "cefal/instances/monoid/basic_types.h"

#include "catch2/catch.hpp"

#include <string>

using namespace cefal;

TEST_CASE("ops::empty() - std::string") {
    CHECK(ops::empty<std::string>() == std::string());
}

TEMPLATE_TEST_CASE("ops::empty() - Sum", "", uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t, float, double, bool, char) {
    CHECK(ops::empty<cefal::Sum<TestType>>() == Approx(TestType(0)));
}

TEMPLATE_TEST_CASE("ops::empty() - Product", "", uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t, float, double, bool,
                   char) {
    CHECK(ops::empty<cefal::Product<TestType>>() == Approx(TestType(1)));
}

TEST_CASE("ops::append() - std::string") {
    SECTION("Lvalue - LValue") {
        const auto left = std::string("abc");
        const auto right = std::string("de");
        std::string result = left | ops::append(right);
        CHECK(result == "abcde");
    }
    SECTION("Lvalue - RValue") {
        const auto left = std::string("abc");
        auto right = std::string("de");
        std::string result = left | ops::append(std::move(right));
        CHECK(result == "abcde");
    }
    SECTION("Rvalue - LValue") {
        auto left = std::string("abc");
        const auto right = std::string("de");
        std::string result = std::move(left) | ops::append(right);
        CHECK(result == "abcde");
    }
    SECTION("Rvalue - RValue") {
        auto left = std::string("abc");
        auto right = std::string("de");
        std::string result = std::move(left) | ops::append(std::move(right));
        CHECK(result == "abcde");
    }
}

TEMPLATE_TEST_CASE("ops::append() - Sum", "", uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t, float, double, bool, char) {
    CHECK((Sum(TestType(5)) | ops::append(Sum(TestType(3)))) == Approx(TestType(8)));
}

TEMPLATE_TEST_CASE("ops::append() - Product", "", uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t, float, double, bool,
                   char) {
    CHECK((Product(TestType(5)) | ops::append(Product(TestType(3)))) == Approx(TestType(15)));
}
