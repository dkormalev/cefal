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
#include <functional>
#include <utility>

namespace cefal {
namespace instances {
template <typename T>
struct Foldable;

namespace detail {
template <typename>
struct FoldableFromFunctionsExists;
// clang-format off
template <typename T, typename InnerT = InnerType_T<T>>
concept HasFoldableMethods = requires(T x, InnerT initial, std::function<InnerT(InnerT, InnerT)> f) {
    typename FoldableFromFunctionsExists<T>::type;
    { std::move(x).foldLeft(std::move(initial), std::move(f)) } -> std::same_as<InnerT>;
};

template <typename T, typename InnerT = InnerType_T<T>>
concept HasFoldableSnakeCaseMethods = requires(T x, InnerT initial, std::function<InnerT(InnerT, InnerT)> f) {
    typename FoldableFromFunctionsExists<T>::type;
    { std::move(x).fold_left(std::move(initial), std::move(f)) } -> std::same_as<InnerT>;
};
// clang-format on
} // namespace detail
} // namespace instances

namespace concepts {
// clang-format off
template <typename T, typename CleanT = std::remove_cvref_t<T>, typename InnerT = InnerType_T<CleanT>,
          typename ConstInnerT = ConstInnerType_T<CleanT>>
concept Foldable =
requires(CleanT x, InnerT init, std::function<InnerT(InnerT, InnerT)> converter, std::function<InnerT(InnerT, ConstInnerT)> constConverter) {
    { instances::Foldable<CleanT>::foldLeft(x, init, std::move(constConverter)) } -> std::same_as<InnerT>;
    { instances::Foldable<CleanT>::foldLeft(std::move(x), std::move(init), std::move(converter)) } -> std::same_as<InnerT>;
};
// clang-format on
} // namespace concepts

namespace ops {
template <typename Result, typename Func>
struct foldLeft {
    foldLeft(Result&& initial, Func&& func) : initial(std::move(initial)), func(std::move(func)) {}
    foldLeft(const Result& initial, Func&& func) : initial(initial), func(std::move(func)) {}
    foldLeft(Result&& initial, const Func& func) : initial(std::move(initial)), func(func) {}
    foldLeft(const Result& initial, const Func& func) : initial(initial), func(func) {}

    template <concepts::Foldable F>
    auto operator()(F&& left) && {
        return instances::Foldable<std::remove_cvref_t<F>>::foldLeft(std::forward<F>(left), std::move(initial), std::move(func));
    }
    template <concepts::Foldable F>
    auto operator()(F&& left) const& {
        return instances::Foldable<std::remove_cvref_t<F>>::foldLeft(std::forward<F>(left), initial, func);
    }

private:
    Result initial;
    Func func;
};

template <typename Result, typename Func>
foldLeft(Result&&, Func &&) -> foldLeft<std::remove_cvref_t<Result>, std::remove_cvref_t<Func>>;

} // namespace ops
} // namespace cefal
