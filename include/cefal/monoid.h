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

#include <concepts>
#include <functional>
#include <utility>

namespace cefal {
namespace helpers {
template <typename T>
struct LightWrapper;
}

namespace instances {
template <typename T>
struct Monoid;

namespace detail {
template<typename>
struct MonoidFromFunctionsExists;
// clang-format off
template <typename T>
concept HasMonoidMethods = requires(T t1, T t2) {
    typename MonoidFromFunctionsExists<T>::type;
    { T::empty() } -> std::same_as<T>;
    { t1.append(std::move(t2)) } -> std::same_as<T>;
};
// clang-format on
} // namespace detail
} // namespace instances

namespace concepts {
// clang-format off
template <typename T, typename CleanT = std::remove_cvref_t<T>>
concept Monoid =
requires(CleanT x1, CleanT x2) {
    { instances::Monoid<CleanT>::empty() } -> std::same_as<CleanT>;
    { instances::Monoid<CleanT>::append(x1, x2) } -> std::same_as<CleanT>;
    { instances::Monoid<CleanT>::append(std::move(x1), std::move(x2)) } -> std::same_as<CleanT>;
};

template <typename T, typename InnerT = InnerType_T<std::remove_cvref_t<T>>, typename CleanT = std::remove_cvref_t<T>>
concept MonoidWithLightWrapper =
Monoid<T> && requires (CleanT x, helpers::LightWrapper<CleanT> wrapper) {
    { instances::Monoid<CleanT>::append(std::move(x), std::move(wrapper)) } -> std::same_as<CleanT>;
};
// clang-format on
} // namespace concepts

namespace ops {
template <concepts::Monoid M>
inline M empty() {
    return instances::Monoid<M>::empty();
}

template <typename T>
struct append;

template <concepts::Monoid M>
struct append<M> {
    append(M&& right) : right(std::move(right)) {}
    append(const M& right) : right(right) {}

    inline auto operator()(M&& left) && { return instances::Monoid<M>::append(std::move(left), std::move(right)); }
    inline auto operator()(const M& left) && { return instances::Monoid<M>::append(left, std::move(right)); }
    inline auto operator()(M&& left) const& { return instances::Monoid<M>::append(std::move(left), right); }
    inline auto operator()(const M& left) const& { return instances::Monoid<M>::append(left, right); }

private:
    M right;
};

template <concepts::Monoid M>
struct append<helpers::LightWrapper<M>> {
    append(helpers::LightWrapper<M>&& right) : right(std::move(right)) {}
    append(const helpers::LightWrapper<M>& right) : right(right) {}

    inline auto operator()(M&& left) && { return instances::Monoid<M>::append(std::move(left), std::move(right)); }
    inline auto operator()(const M& left) && { return instances::Monoid<M>::append(left, std::move(right)); }
    inline auto operator()(M&& left) const& { return instances::Monoid<M>::append(std::move(left), right); }
    inline auto operator()(const M& left) const& { return instances::Monoid<M>::append(left, right); }

private:
    helpers::LightWrapper<M> right;
};

template <typename M>
append(M &&) -> append<std::remove_cvref_t<M>>;
template <typename M>
append(const helpers::LightWrapper<M>&) -> append<helpers::LightWrapper<M>>;
template <typename M>
append(helpers::LightWrapper<M>&&) -> append<helpers::LightWrapper<M>>;

} // namespace ops
} // namespace cefal
