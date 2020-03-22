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
#include "cefal/filterable.h"
#include "cefal/foldable.h"
#include "cefal/functor.h"
#include "cefal/monoid.h"

#include <algorithm>
#include <type_traits>

namespace cefal::instances {
namespace detail {
template <typename Src, typename Func>
concept StdRemoveIfable = requires(Src src, Func func, InnerType_T<Src> value) {
    {*src.begin() = value};
    {src.erase(std::remove_if(src.begin(), src.end(), func), src.end())};
};

template <typename Src>
concept Erasable = requires(Src src, InnerType_T<Src> value) {
    {src.erase(value)};
    {src.erase(src.begin())};
};
} // namespace detail

template <typename Src>
// clang-format off
requires concepts::Foldable<Src> && concepts::Monoid<Src> && (!detail::HasFilterableMethods<Src>)
// clang-format on
struct Filterable<Src> {
private:
    using T = InnerType_T<Src>;

public:
    template <typename Input, typename Func>
    // clang-format off
    requires std::same_as<std::remove_cvref_t<Input>, Src> && (!concepts::SingletonEnabledMonoid<Src>) && concepts::Functor<Src>
    // clang-format on
    static auto filter(Input&& src, Func&& func) {
        return std::forward<Input>(src)
               | ops::foldLeft(ops::empty<Src>(), [func = std::forward<Func>(func)]<typename T2>(Src&& l, T2&& r) {
                     if (!func(r))
                         return std::move(l);
                     return std::move(l) | ops::append(ops::unit<Src>(std::forward<T2>(r)));
                 });
    }

    // We have SingletonEnabled variant divided into rvalue and lvalue here
    // to provide optimization for rvalue std container
    template <typename Func>
    // clang-format off
    requires concepts::SingletonEnabledMonoid<Src>
        // clang-format on
        static auto filter(const Src& src, Func&& func) {
        using Dest = WithInnerType_T<Src, std::invoke_result_t<Func, T>>;
        return src | ops::foldLeft(ops::empty<Src>(), [func = std::forward<Func>(func)](Src&& l, const T& r) {
                   if (!func(r))
                       return std::move(l);
                   return std::move(l) | ops::append(helpers::SingletonFrom<Src>{r});
               });
    }

    template <typename Func>
    // clang-format off
    requires concepts::SingletonEnabledMonoid<Src>
        // clang-format on
        static auto filter(Src&& src, Func&& func) {
        using Dest = WithInnerType_T<Src, std::invoke_result_t<Func, T>>;
        return std::move(src) | ops::foldLeft(ops::empty<Src>(), [func = std::forward<Func>(func)](Src&& l, T&& r) {
                   if (!func(r))
                       return std::move(l);
                   return std::move(l) | ops::append(helpers::SingletonFrom<Src>{std::move(r)});
               });
    }

    // We don't need SingletonEnabledMonoid here, but it makes code easier (no extra negative checks and all that)
    // Technically it shouldn't occur, because SingletonEnabledMonoid is less strict than StdRemoveIfable
    // We also can't use std::erase_if here due to it being in multiple headers, so we need an extra overload for set-like containers
    template <typename Func>
    // clang-format off
    requires concepts::SingletonEnabledMonoid<Src> && detail::StdRemoveIfable<Src, Func>
        // clang-format on
        static auto filter(Src&& src, Func&& func) {
        src.erase(std::remove_if(src.begin(), src.end(), [func = std::forward<Func>(func)](const T& r) { return !func(r); }),
                  src.end());
        return std::move(src);
    }

    template <typename Func>
    // clang-format off
    requires concepts::SingletonEnabledMonoid<Src> && (!detail::StdRemoveIfable<Src, Func>) && detail::Erasable<Src>
        // clang-format on
        static auto filter(Src&& src, Func&& func) {
        for (auto it = src.begin(); it != src.end();) {
            if (func(*it))
                ++it;
            else
                it = src.erase(it);
        }
        return std::move(src);
    }
};
} // namespace cefal::instances
