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

#include "cefal/detail/instantiator.h"

#include "cefal/common.h"

#include <concepts>
#include <functional>
#include <utility>

namespace cefal {
namespace instances {
template <typename T, typename U>
struct Converter;
} // namespace instances

namespace concepts {
// clang-format off
template <typename T, typename U>
concept CanConvert = requires(T x) {
    { instances::Converter<T, U>::convert(x) } -> std::same_as<U>;
    { instances::Converter<T, U>::convert(std::move(x)) } -> std::same_as<U>;
};
// clang-format on
} // namespace concepts

namespace ops {
namespace detail {
template <template <typename...> typename U>
struct as_templated {
    template <typename T, typename CleanT = std::remove_cvref_t<T>,
              typename FullU = WithInnerType_T<cefal::detail::Instantiator_T<U>, NakedInnerType_T<CleanT>>>
    requires concepts::CanConvert<CleanT, FullU> auto operator()(T&& left) const {
        return instances::Converter<CleanT, FullU>::convert(std::forward<T>(left));
    }
};
template <typename U>
struct as_full {
    template <typename T, typename CleanT = std::remove_cvref_t<T>, typename CleanU = std::remove_cvref_t<U>>
    requires concepts::CanConvert<CleanT, CleanU> auto operator()(T&& left) const {
        static_assert(std::is_same_v<NakedInnerType_T<CleanU>, NakedInnerType_T<CleanT>>,
                      "cefal::ops::as can be called only for destination with same inner type as source");
        return instances::Converter<CleanT, CleanU>::convert(std::forward<T>(left));
    }
};
} // namespace detail

template <template <typename...> typename U>
inline auto as() {
    return detail::as_templated<U>();
}
template <typename U>
inline auto as() {
    return detail::as_full<U>();
}

} // namespace ops
} // namespace cefal
