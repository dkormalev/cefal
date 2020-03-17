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

#include "cefal/detail/type_traits.h"
#include "cefal/functor.h"

#include <algorithm>
#include <type_traits>

namespace cefal::instances {
namespace detail {
// clang-format off
template <typename T>
concept HasFunctorMethods = requires(T t, cefal::detail::InnerType_T<T> value,
                                     std::function<cefal::detail::InnerType_T<T>(cefal::detail::InnerType_T<T>)> f) {
    { T::unit(std::move(value)) } -> std::same_as<T>;
    { t.map(std::move(f)) } -> std::same_as<T>;
};
// clang-format on
} // namespace detail

template <detail::HasFunctorMethods T>
struct Functor<T> {
    template <typename Inner>
    static T unit(Inner&& x) {
        return T::unit(std::forward<Inner>(x));
    }

    template <typename Func>
    static auto map(const T& src, Func&& func) {
        return src.map(std::forward<Func>(func));
    }
};
} // namespace cefal::instances
