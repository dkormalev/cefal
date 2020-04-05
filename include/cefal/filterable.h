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
namespace instances {
template <typename T>
struct Filterable;

namespace detail {
template <typename>
struct FilterableFromFunctionsExists;
// clang-format off
template <typename T, typename InnerT = InnerType_T<T>>
concept HasFilterableMethods = requires(T x, std::function<bool(InnerT)> predicate) {
    typename FilterableFromFunctionsExists<T>::type;
    { x.filter(std::move(predicate)) } -> cefal::detail::ValidInnerTypeTransformationFrom<T>;
};
// clang-format on
} // namespace detail
} // namespace instances

namespace concepts {
// clang-format off
template <typename T, typename InnerT = InnerType_T<std::remove_cvref_t<T>>, typename CleanT = std::remove_cvref_t<T>>
concept Filterable =
requires(CleanT x, std::function<bool(InnerT)> predicate) {
    { instances::Filterable<CleanT>::filter(x, std::move(predicate)) } -> cefal::detail::ValidInnerTypeTransformationFrom<CleanT>;
    { instances::Filterable<CleanT>::filter(std::move(x), std::move(predicate)) } -> cefal::detail::ValidInnerTypeTransformationFrom<CleanT>;
};
// clang-format on
} // namespace concepts

namespace ops {
template <typename Func>
struct filter {
    filter(Func&& func) : func(std::move(func)) {}
    filter(const Func& func) : func(func) {}

    template <concepts::Filterable F>
    auto operator()(F&& left) && {
        return instances::Filterable<std::remove_cvref_t<F>>::filter(std::forward<F>(left), std::move(func));
    }
    template <concepts::Filterable F>
    auto operator()(F&& left) const& {
        return instances::Filterable<std::remove_cvref_t<F>>::filter(std::forward<F>(left), func);
    }

private:
    Func func;
};

template <typename Func>
struct innerFilter {
    innerFilter(Func&& func) : func(std::move(func)) {}
    innerFilter(const Func& func) : func(func) {}

    template <concepts::Functor F, concepts::Filterable InnerF = InnerType_T<std::remove_cvref_t<F>>>
    auto operator()(F&& left) const& {
        return std::forward<F>(left) | map([this]<typename T>(T&& x) { return std::forward<T>(x) | filter(func); });
    }

private:
    Func func;
};

template <typename Func>
filter(Func &&) -> filter<std::remove_cvref_t<Func>>;
template <typename Func>
innerFilter(Func &&) -> innerFilter<std::remove_cvref_t<Func>>;

} // namespace ops
} // namespace cefal
