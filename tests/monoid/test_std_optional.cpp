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
#include "cefal/instances/monoid/std_optional.h"

#include "catch2/catch.hpp"

#include <string>

using namespace cefal;

TEST_CASE("ops::empty()") {
    std::optional<std::string> result = ops::empty<std::optional<std::string>>();
    CHECK(!result);
}

TEST_CASE("ops::append() - Both") {
    std::optional<std::string> result = std::optional<std::string>("abc") | ops::append(std::optional<std::string>("de"));
    REQUIRE(result);
    CHECK(*result == "abcde");
}

TEST_CASE("ops::append() - Left") {
    std::optional<std::string> result = std::optional<std::string>("abc") | ops::append(std::optional<std::string>());
    REQUIRE(result);
    CHECK(*result == "abc");
}

TEST_CASE("ops::append() - Right") {
    std::optional<std::string> result = std::optional<std::string>() | ops::append(std::optional<std::string>("de"));
    REQUIRE(result);
    CHECK(*result == "de");
}

TEST_CASE("ops::append() - None") {
    std::optional<std::string> result = std::optional<std::string>() | ops::append(std::optional<std::string>());
    CHECK(!result);
}
