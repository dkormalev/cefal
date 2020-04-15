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

#include "cefal/detail/std_concepts.h"

#include "cefal/common.h"
#include "cefal/foldable.h"

#include <numeric>
#include <type_traits>

namespace cefal::instances {
template <cefal::detail::SingleSocketedStdContainer Src>
struct Foldable<Src> {
    template <typename Result, typename Func>
    static auto foldLeft(const Src& src, Result&& initial, Func&& func) {
        return std::accumulate(src.begin(), src.end(), std::forward<Result>(initial), std::forward<Func>(func));
    }

    template <typename Result, typename Func>
    static auto foldLeft(Src&& src, Result&& initial, Func&& func) requires cefal::detail::VectorLikeContainer<Src> {
        using CleanResult = std::remove_cvref_t<Result>;
        CleanResult result = std::forward<Result>(initial);
        for (auto&& x : src)
            result = func(std::move(result), std::move(x));
        return result;
    }

    template <typename Result, typename Func>
    static auto foldLeft(Src&& src, Result&& initial, Func&& func) requires cefal::detail::SetLikeContainer<Src> {
        using CleanResult = std::remove_cvref_t<Result>;
        using InnerT = typename Src::value_type;
        CleanResult result = std::forward<Result>(initial);
        if constexpr (std::is_trivial_v<InnerT> && sizeof(InnerT) <= 8) {
            for (auto x : src)
                result = func(std::move(result), std::move(x));
        } else {
            while (!src.empty())
                result = func(std::move(result), std::move(src.extract(src.begin()).value()));
        }
        return result;
    }
};

template <cefal::detail::DoubleSocketedStdContainer Src>
struct Foldable<Src> {
    template <typename Result, typename Func>
    static auto foldLeft(const Src& src, Result&& initial, Func&& func) {
        using CleanResult = std::remove_cvref_t<Result>;
        CleanResult result = std::forward<Result>(initial);
        for (const auto& x : src)
            result = func(std::move(result), std::forward_as_tuple(x.first, x.second));
        return result;
    }

    template <typename Result, typename Func>
    static auto foldLeft(Src&& src, Result&& initial, Func&& func) {
        using CleanResult = std::remove_cvref_t<Result>;
        using InnerT = typename Src::value_type;
        using KeyT = std::tuple_element_t<0, InnerT>;
        CleanResult result = std::forward<Result>(initial);
        if constexpr (std::is_trivial_v<KeyT> && sizeof(KeyT) <= 8) {
            for (auto& x : src) {
                KeyT key = x.first;
                result = func(std::move(result), std::forward_as_tuple(key, x.second));
            }
        } else {
            while (!src.empty()) {
                auto node = src.extract(src.begin());
                result = func(std::move(result), std::forward_as_tuple(node.key(), node.mapped()));
            }
        }
        return result;
    }
};
} // namespace cefal::instances
