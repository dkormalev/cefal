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

TEMPLATE_PRODUCT_TEST_CASE("ops::empty()", "",
                           (std::vector, std::list, std::deque, std::set, std::unordered_set, std::multiset,
                            std::unordered_multiset),
                           (int, std::string)) {
    TestType result = ops::empty<TestType>();
    CHECK(result.empty());
}

TEMPLATE_PRODUCT_TEST_CASE("ops::append() - Both", "",
                           (std::vector, std::list, std::deque, std::set, std::unordered_set, std::multiset,
                            std::unordered_multiset),
                           (int, std::string)) {
    using InnerType = typename TestType::value_type;
    TestType result;
    SECTION("Lvalue - LValue") {
        const auto left = TestType{createValue<InnerType>(1)};
        const auto right = TestType{createValue<InnerType>(2)};
        result = left | ops::append(right);
    }
    SECTION("Lvalue - RValue") {
        const auto left = TestType{createValue<InnerType>(1)};
        auto right = TestType{createValue<InnerType>(2)};
        result = left | ops::append(std::move(right));
    }
    SECTION("Rvalue - LValue") {
        auto left = TestType{createValue<InnerType>(1)};
        const auto right = TestType{createValue<InnerType>(2)};
        result = std::move(left) | ops::append(right);
    }
    SECTION("Rvalue - RValue") {
        auto left = TestType{createValue<InnerType>(1)};
        auto right = TestType{createValue<InnerType>(2)};
        result = std::move(left) | ops::append(std::move(right));
    }
    CHECK(result == TestType{createValue<InnerType>(1), createValue<InnerType>(2)});
}

TEMPLATE_PRODUCT_TEST_CASE("ops::append() - Left", "",
                           (std::vector, std::list, std::deque, std::set, std::unordered_set, std::multiset,
                            std::unordered_multiset),
                           (int, std::string)) {
    using InnerType = typename TestType::value_type;
    TestType result = TestType{createValue<InnerType>(1)} | ops::append(TestType());
    CHECK(result == TestType{createValue<InnerType>(1)});
}

TEMPLATE_PRODUCT_TEST_CASE("ops::append() - Right", "",
                           (std::vector, std::list, std::deque, std::set, std::unordered_set, std::multiset,
                            std::unordered_multiset),
                           (int, std::string)) {
    using InnerType = typename TestType::value_type;
    TestType result = TestType() | ops::append(TestType{createValue<InnerType>(2)});
    CHECK(result == TestType{createValue<InnerType>(2)});
}

TEMPLATE_PRODUCT_TEST_CASE("ops::append() - None", "",
                           (std::vector, std::list, std::deque, std::set, std::unordered_set, std::multiset,
                            std::unordered_multiset),
                           (int, std::string)) {
    using InnerType = typename TestType::value_type;
    TestType result = TestType() | ops::append(TestType());
    REQUIRE(result.empty());
}
