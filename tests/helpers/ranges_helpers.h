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

#include "cefal/helpers/std_ranges.h"

#include <ranges>
#include <vector>

struct Transform {
    template <typename T>
    friend auto operator|(T&& prev, Transform) {
        return std::forward<T>(prev) | std::views::transform([](auto x) { return x; });
    }
};
struct Filter {
    template <typename T>
    friend auto operator|(T&& prev, Filter) {
        return std::forward<T>(prev) | std::views::filter([](auto) { return true; });
    }
};
struct Take {
    template <typename T>
    friend auto operator|(T&& prev, Take) {
        return std::forward<T>(prev) | std::views::take(10);
    }
};
struct TakeWhile {
    template <typename T>
    friend auto operator|(T&& prev, TakeWhile) {
        return std::forward<T>(prev) | std::views::take_while([](auto) { return true; });
    }
};
struct Drop {
    template <typename T>
    friend auto operator|(T&& prev, Drop) {
        return std::forward<T>(prev) | std::views::drop(0);
    }
};
struct DropWhile {
    template <typename T>
    friend auto operator|(T&& prev, DropWhile) {
        return std::forward<T>(prev) | std::views::drop_while([](auto) { return false; });
    }
};
struct SplitAndJoin {
    template <typename T>
    friend auto operator|(T&& prev, SplitAndJoin) {
        return std::views::join(std::forward<T>(prev) | std::views::split(std::string()));
    }
};
struct Reverse {
    template <typename T>
    friend auto operator|(T&& prev, Reverse) {
        return std::views::reverse(std::forward<T>(prev));
    }
};
struct Own {
    template <typename T>
    friend auto operator|(T&& prev, Own) {
        return std::forward<T>(prev) | cefal::ops::ownView();
    }
};

template <typename... T>
struct HasOp;

template <typename Op, typename... Ts>
struct HasOp<Op, Op, Ts...> : std::true_type {};

template <typename Op, typename Head, typename... Ts>
struct HasOp<Op, Head, Ts...> : HasOp<Op, Ts...> {};

template <typename Op>
struct HasOp<Op> : std::false_type {};

template <std::ranges::view V>
auto materialize(V&& view) {
    std::vector<cefal::InnerType_T<V>> result;
    for (auto&& x : view)
        result.push_back(x);
    return result;
}
