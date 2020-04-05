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

#include <ranges>

namespace cefal {
namespace detail {
template <std::ranges::view To, std::ranges::view From>
struct IsValidInnerTypeTransformation<To, From> : std::true_type {};
} // namespace detail

template <std::ranges::view T>
struct OwnedView : public std::ranges::view_interface<OwnedView<T>> {
private:
    T _base;

public:
    OwnedView() = default;
    OwnedView(T t) : _base(std::move(t)) {}
    constexpr auto begin() const { return std::ranges::begin(_base); }
    constexpr auto end() const { return std::ranges::end(_base); }

    constexpr auto cbegin() const requires requires { std::ranges::cbegin(_base); }
    { return std::ranges::cbegin(_base); }
    constexpr auto cend() const requires requires { std::ranges::cend(_base); }
    { return std::ranges::cend(_base); }

    constexpr auto rbegin() const requires requires { std::ranges::rbegin(_base); }
    { return std::ranges::rbegin(_base); }
    constexpr auto rend() const requires requires { std::ranges::rend(_base); }
    { return std::ranges::rend(_base); }

    constexpr bool empty() const requires requires { std::ranges::empty(_base); }
    { return std::ranges::empty(_base); }

    constexpr auto size() const requires std::ranges::sized_range<T> { return std::ranges::size(_base); }

    constexpr auto data() const requires std::ranges::contiguous_range<T> { return std::ranges::data(_base); }
};

template <typename>
struct IsOwnedView : std::false_type {};

template <typename T>
struct IsOwnedView<OwnedView<T>> : std::true_type {};

template <typename T>
// clang-format off
requires requires (T t) {{ t.base() } -> std::ranges::view<>; }
// clang-format on
struct IsOwnedView<T> {
    static constexpr bool value = IsOwnedView<decltype(T().base())>::value;
};

template <typename T>
constexpr inline bool IsOwnedView_V = IsOwnedView<T>::value;

template <std::ranges::view C>
struct InnerType<C> {
    using type = std::remove_cvref_t<std::ranges::range_value_t<C>>;
};

namespace ops {
struct ownView {
    template <typename T>
    requires std::ranges::view<std::remove_cvref_t<T>> OwnedView<std::remove_cvref_t<T>> operator()(T&& left) const& {
        if constexpr (IsOwnedView_V<std::remove_cvref_t<T>>) {
            return std::forward<T>(left);
        } else {
            return OwnedView(std::forward<T>(left));
        }
    }
};

} // namespace ops
} // namespace cefal
