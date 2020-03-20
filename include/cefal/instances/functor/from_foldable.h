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
#include "cefal/foldable.h"
#include "cefal/functor.h"
#include "cefal/monoid.h"

#include <algorithm>
#include <type_traits>

namespace cefal::instances {
template <typename Src>
requires concepts::Foldable<Src> && concepts::Monoid<Src> && (!concepts::MonoidWithLightWrapper<Src>) && (!detail::HasFunctorMethods<Src>)
struct Functor<Src> {
private:
    using T = InnerType_T<Src>;

public:
    static Src unit(const T& x) { return Src{x}; }
    static Src unit(T&& x) { return Src{std::move(x)}; }

    template <typename Func>
    static auto map(const Src& src, Func&& func) {
        using Dest = WithInnerType_T<Src, std::invoke_result_t<Func, T>>;
        return instances::Foldable<Src>::foldLeft(src,
                                                  instances::Monoid<Dest>::empty(),
                                                  [func = std::forward<Func>(func)](Dest&& l, const T& r) {
                                                      return std::move(l) | ops::append(unit(func(r)));
                                                  });
    }

    template <typename Func>
    static auto map(Src&& src, Func&& func) {
        using Dest = WithInnerType_T<Src, std::invoke_result_t<Func, T>>;
        using DestMonoid = instances::Monoid<Dest>;
        return instances::Foldable<Src>::foldLeft(std::move(src),
                                                  instances::Monoid<Dest>::empty(),
                                                  [func = std::forward<Func>(func)](Dest&& l, T&& r) {
                                                      return std::move(l) | ops::append(unit(func(std::move(r))));
                                                  });
    }
};

template <typename Src>
requires concepts::Foldable<Src> && concepts::MonoidWithLightWrapper<Src> && (!detail::HasFunctorMethods<Src>)
struct Functor<Src> {
private:
    using T = InnerType_T<Src>;

public:
    static Src unit(const T& x) { return Src{x}; }
    static Src unit(T&& x) { return Src{std::move(x)}; }

    template <typename Func>
    static auto map(const Src& src, Func&& func) {
        using Dest = WithInnerType_T<Src, std::invoke_result_t<Func, T>>;
        return instances::Foldable<Src>::foldLeft(src,
                                                  instances::Monoid<Dest>::empty(),
                                                  [func = std::forward<Func>(func)](Dest&& l, const T& r) {
                                                      return std::move(l) | ops::append(helpers::LightWrapper<Dest>{func(r)});
                                                  });
    }

    template <typename Func>
    static auto map(Src&& src, Func&& func) {
        using Dest = WithInnerType_T<Src, std::invoke_result_t<Func, T>>;
        return instances::Foldable<Src>::foldLeft(std::move(src),
                                                  instances::Monoid<Dest>::empty(),
                                                  [func = std::forward<Func>(func)](Dest&& l, T&& r) {
                                                      return std::move(l)
                                                             | ops::append(helpers::LightWrapper<Dest>{func(std::move(r))});
                                                  });
    }
};
} // namespace cefal::instances
