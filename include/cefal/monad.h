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
#include "cefal/functor.h"

#include <concepts>
#include <functional>
#include <utility>

namespace cefal {
namespace instances {
template <typename T>
struct Monad;

namespace detail {
template <typename>
struct MonadFromFunctionsExists;
// clang-format off
template <typename T, typename InnerT = InnerType_T<T>>
concept HasMonadMethods = requires(T x, std::function<T(InnerT)> f) {
    typename MonadFromFunctionsExists<T>::type;
    { std::move(x).flatMap(std::move(f)) } -> cefal::detail::ValidInnerTypeTransformationFrom<T>;
};

template <typename T, typename InnerT = InnerType_T<T>>
concept HasMonadSnakeCaseMethods = requires(T x, std::function<T(InnerT)> f) {
    typename MonadFromFunctionsExists<T>::type;
    { std::move(x).flat_map(std::move(f)) } -> cefal::detail::ValidInnerTypeTransformationFrom<T>;
};
// clang-format on
} // namespace detail
} // namespace instances

namespace concepts {
// clang-format off
template <typename T, typename CleanT = std::remove_cvref_t<T>, typename InnerT = InnerType_T<CleanT>,
          typename ConstInnerT = ConstInnerType_T<CleanT>>
concept Monad =
Functor<T> && requires (CleanT x, std::function<CleanT(InnerT)> converter, std::function<CleanT(ConstInnerT)> constConverter) {
  {instances::Monad<CleanT>::flatMap(x, std::move(constConverter))} -> cefal::detail::ValidInnerTypeTransformationFrom<CleanT>;
  {instances::Monad<CleanT>::flatMap(std::move(x), std::move(converter))} -> cefal::detail::ValidInnerTypeTransformationFrom<CleanT>;
};
// clang-format on
} // namespace concepts

namespace ops {
template <typename Func>
struct flatMap {
    flatMap(Func&& func) : func(std::move(func)) {}
    flatMap(const Func& func) : func(func) {}

    template <concepts::Monad M>
    auto operator()(M&& left) && {
        return instances::Monad<std::remove_cvref_t<M>>::flatMap(std::forward<M>(left), std::move(func));
    }
    template <concepts::Monad M>
    auto operator()(M&& left) const& {
        return instances::Monad<std::remove_cvref_t<M>>::flatMap(std::forward<M>(left), func);
    }

private:
    Func func;
};

template <typename Func>
struct innerFlatMap {
    innerFlatMap(Func&& func) : func(std::move(func)) {}
    innerFlatMap(const Func& func) : func(func) {}

    template <concepts::Functor F, concepts::Monad InnerM = InnerType_T<std::remove_cvref_t<F>>>
    auto operator()(F&& left) const& {
        return std::forward<F>(left) | map([this]<typename T>(T&& x) { return std::forward<T>(x) | flatMap(func); });
    }

private:
    Func func;
};

template <typename Func>
flatMap(Func &&) -> flatMap<std::remove_cvref_t<Func>>;
template <typename Func>
innerFlatMap(Func &&) -> innerFlatMap<std::remove_cvref_t<Func>>;

} // namespace ops
} // namespace cefal
