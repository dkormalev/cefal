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
#include "cefal/converter.h"
#include "cefal/helpers/std_ranges.h"

#include <numeric>
#include <ranges>
#include <type_traits>

namespace cefal::instances {
namespace detail {

template <typename Dest, std::ranges::sized_range Src>
requires cefal::detail::TransferableSize<Src, Dest> void prepareConvertFromRangeDestination(const Src& src, Dest& dest) {
    dest.reserve(src.size());
}

template <typename Dest, typename Src>
void prepareConvertFromRangeDestination(const Src& src, Dest& dest) {
}

template <cefal::detail::StdContainer Dest, typename Src>
Dest createConvertFromRangeDestination(const Src& src) {
    Dest dest;
    prepareConvertFromRangeDestination(src, dest);
    return dest;
}

template <cefal::detail::SetLikeContainer Dest, typename T>
void addToConvertFromRangeDestination(Dest& dest, T&& x) {
    dest.insert(std::forward<T>(x));
}

template <cefal::detail::VectorLikeContainer Dest, typename T>
void addToConvertFromRangeDestination(Dest& dest, T&& x) {
    dest.push_back(std::forward<T>(x));
}

template <cefal::detail::DoubleSocketedStdContainer Dest, typename K, typename V>
void addToConvertFromRangeDestination(Dest& dest, const std::pair<K, V>& x) {
    dest.insert(x);
}
template <cefal::detail::DoubleSocketedStdContainer Dest, typename K, typename V>
void addToConvertFromRangeDestination(Dest& dest, std::pair<K, V>&& x) {
    dest.insert(std::move(x));
}
template <cefal::detail::DoubleSocketedStdContainer Dest, typename K, typename V>
void addToConvertFromRangeDestination(Dest& dest, const std::tuple<K, V>& x) {
    dest.emplace(std::get<0>(x), std::get<1>(x));
}
template <cefal::detail::DoubleSocketedStdContainer Dest, typename K, typename V>
void addToConvertFromRangeDestination(Dest& dest, std::tuple<K, V>&& x) {
    dest.emplace(std::move(std::get<0>(x)), std::move(std::get<1>(x)));
}
} // namespace detail

template <std::ranges::range Src, cefal::detail::StdContainer Dest>
struct Converter<Src, Dest> {
    template <typename = void>
    requires cefal::detail::VectorLikeContainer<Src> static auto convert(Src&& src) {
        Dest dest = detail::createConvertFromRangeDestination<Dest>(src);
        for (auto&& x : src)
            detail::addToConvertFromRangeDestination(dest, std::move(x));
        return dest;
    }

    template <typename = void>
    requires cefal::detail::SetLikeContainer<Src> static auto convert(Src&& src) {
        using InnerT = std::ranges::range_value_t<Src>;
        Dest dest = detail::createConvertFromRangeDestination<Dest>(src);
        if constexpr (std::is_trivial_v<InnerT> && sizeof(InnerT) <= 8) {
            for (auto x : src)
                detail::addToConvertFromRangeDestination(dest, std::move(x));
        } else {
            while (!src.empty())
                detail::addToConvertFromRangeDestination(dest, std::move(src.extract(src.begin()).value()));
        }
        return dest;
    }

    template <typename = void>
    requires cefal::detail::DoubleSocketedStdContainer<Src> static auto convert(Src&& src) {
        using InnerT = std::ranges::range_value_t<Src>;
        using KeyT = std::remove_cvref_t<std::tuple_element_t<0, InnerT>>;
        Dest dest = detail::createConvertFromRangeDestination<Dest>(src);
        if constexpr (std::is_trivial_v<KeyT> && sizeof(KeyT) <= 8) {
            for (auto& x : src) {
                KeyT key = x.first;
                detail::addToConvertFromRangeDestination(dest, std::make_pair(key, std::move(x.second)));
            }
        } else {
            while (!src.empty()) {
                auto node = src.extract(src.begin());
                detail::addToConvertFromRangeDestination(dest, std::make_pair(std::move(node.key()), std::move(node.mapped())));
            }
        }
        return dest;
    }

    static auto convert(const Src& src) {
        Dest dest = detail::createConvertFromRangeDestination<Dest>(src);
        for (const auto& x : src)
            detail::addToConvertFromRangeDestination(dest, x);
        return dest;
    }
};

} // namespace cefal::instances
