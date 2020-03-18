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
struct Functor;
} // namespace instances

namespace concepts {
// clang-format off
template <typename F, typename InnerT = InnerType_T<std::remove_cvref_t<F>>, typename CleanF = std::remove_cvref_t<F>>
concept Functor =
requires(CleanF f, InnerT value, std::function<InnerT(InnerT)> converter) {
    { instances::Functor<CleanF>::unit(value) } -> std::same_as<CleanF>;
    { instances::Functor<CleanF>::unit(std::move(value)) } -> std::same_as<CleanF>;
    { instances::Functor<CleanF>::map(f, std::move(converter)) } -> std::same_as<CleanF>;
    { instances::Functor<CleanF>::map(std::move(f), std::move(converter)) } -> std::same_as<CleanF>;
};
// clang-format on
} // namespace concepts

namespace ops {
template <concepts::Functor F, typename T>
inline F unit(T&& x) {
    return instances::Functor<F>::unit(std::forward<T>(x));
}

template <template <typename...> typename F, typename T, typename CleanT = std::remove_cvref_t<T>>
inline auto unit(T&& x) requires concepts::Functor<F<CleanT>> {
    return unit<F<CleanT>>(std::forward<T>(x));
}

template <typename Func>
struct map {
    map(Func&& func) : func(std::move(func)) {}
    map(const Func& func) : func(func) {}

    template <concepts::Functor F>
    auto operator()(F&& left) && {
        return instances::Functor<std::remove_cvref_t<F>>::map(std::forward<F>(left), std::move(func));
    }
    template <concepts::Functor F>
    auto operator()(F&& left) const& {
        return instances::Functor<std::remove_cvref_t<F>>::map(std::forward<F>(left), func);
    }

private:
    Func func;
};

template <typename Func>
struct innerMap {
    innerMap(Func&& func) : func(std::move(func)) {}
    innerMap(const Func& func) : func(func) {}

    template <concepts::Functor F, concepts::Functor InnerF = InnerType_T<std::remove_cvref_t<F>>>
    auto operator()(F&& left) const& {
        return std::forward<F>(left) | map([this]<typename T>(T&& x) { return std::forward<T>(x) | map(func); });
    }

private:
    Func func;
};

} // namespace ops
} // namespace cefal
