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

#pragma once

#include "cefal/common.h"
#include "cefal/monoid.h"

#include <concepts>
#include <string>
#include <type_traits>

namespace cefal::instances {
template <>
struct Monoid<std::string> {
    static std::string empty() { return {}; }

    template <typename T1, typename T2>
    static Src append(T1&& left, T2&& right) {
        static_assert(std::is_same_v<std::remove_cvref_t<T1>, std::string>, "Argument type should be the same as monoid");
        static_assert(std::is_same_v<std::remove_cvref_t<T2>, std::string>, "Argument type should be the same as monoid");
        return std::forward<T1>(left) + std::forward<T2>(right);
    }
};

template <std::integral T>
struct Monoid<Sum<T>> {
    static Sum<T> empty() { return static_cast<T>(0); }
    static Sum<T> append(Sum<T> left, Sum<T> right) { return left.value + right.value; }
};

template <std::integral T>
struct Monoid<Product<T>> {
    static Product<T> empty() { return static_cast<T>(1); }
    static Product<T> append(Product<T> left, Product<T> right) { return left.value * right.value; }
};
} // namespace cefal::instances
