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
template <typename F>
concept Functor =
requires(F f, detail::InnerType_T<F> value, std::function<detail::InnerType_T<F>(detail::InnerType_T<F>)> converter) {
    { instances::Functor<F>::unit(std::move(value)) } -> std::same_as<F>;
    { instances::Functor<F>::map(f, std::move(converter)) } -> std::same_as<F>;
};
// clang-format on
} // namespace concepts

namespace ops {
template <concepts::Functor F, typename T>
inline F unit(T&& x) {
    return instances::Functor<F>::unit(std::forward<T>(x));
}

template <template <typename...> typename F, typename T>
inline F<T> unit(T&& x) requires concepts::Functor<F<T>> {
    return unit<F<T>>(std::forward<T>(x));
}

template <typename FuncT>
struct map {
    using Func = FuncT;
    map(Func&& func) : func(std::move(func)) {}
    map(const Func& func) : func(func) {}
    template <concepts::Functor F>
    inline auto operator()(const F& f) && {
        return instances::Functor<F>::map(f, std::move(func));
    }

private:
    Func func;
};

template <typename Left, typename Func>
inline auto operator|(const Left& left, map<Func>&& op) {
    return std::move(op)(left);
}
} // namespace ops
} // namespace cefal
