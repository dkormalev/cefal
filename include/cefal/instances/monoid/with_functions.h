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

#include <algorithm>
#include <type_traits>

namespace cefal::instances {
namespace detail {
template <typename T>
struct MonoidFromFunctionsExists {
    using type = T;
};
} // namespace detail
template <detail::HasMonoidMethods T>
struct Monoid<T> {
    static T empty() { return T::empty(); }

    template <typename T1, typename T2>
    static T append(T1&& left, T2&& right) {
        static_assert(std::is_same_v<std::remove_cvref_t<T1>, T>, "Argument type should be the same as monoid");
        static_assert(std::is_same_v<std::remove_cvref_t<T2>, T> || std::is_same_v<std::remove_cvref_t<T2>, helpers::SingletonFrom<T>>,
                      "Argument type should be the same as monoid");
        return std::forward<T1>(left).append(std::forward<T2>(right));
    }
};
} // namespace cefal::instances
