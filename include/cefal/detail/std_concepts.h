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

#include "cefal/detail/common_concepts.h"

#include "cefal/common.h"

#include <concepts>
#include <iterator>

namespace cefal::detail {
// clang-format off
template<typename C>
concept StdContainer = requires (C c) {
    typename C::iterator;
    typename C::value_type;
    {std::begin(c)} -> std::same_as<typename C::iterator>;
    {std::end(c)} -> std::same_as<typename C::iterator>;
    {std::next(std::begin(c))} -> std::same_as<typename C::iterator>;
    {*std::begin(c)} -> std::convertible_to<typename C::value_type>;
    {c.size()} -> std::convertible_to<size_t>;
};

template<typename C>
concept SingleSocketedStdContainer = SingleSocketed<C> && StdContainer<C>;

template<typename C>
concept SetLikeContainer = SingleSocketedStdContainer<C> && requires (C c, InnerType_T<C> value) {
    typename C::node_type;
    c.insert(value);
};

template<typename C>
concept VectorLikeContainer = SingleSocketedStdContainer<C> && requires (C c, InnerType_T<C> value, size_t size) {
    c.push_back(value);
};

template <typename C>
concept Reservable = SingleSocketedStdContainer<C> && requires(C c, size_t size) {
    c.reserve(size);
    c.shrink_to_fit();
};

template <typename Src, typename Dest>
concept TransferableSize = SingleSocketedStdContainer<Src> && Reservable<Dest>;

template <typename Src, typename Func>
concept SelfTransformable = SingleSocketedStdContainer<Src> && requires(Src src, Func func, InnerType_T<Src> value) {
    { func(std::move(value)) } -> std::same_as<InnerType_T<Src>>;
    *src.begin() = value;
    std::transform(src.begin(), src.end(), src.begin(), func); // Technically we need loop here, but it should suffice too
};

template <typename Src, typename Func>
concept StdRemoveIfable = SingleSocketedStdContainer<Src> && requires(Src src, Func func, InnerType_T<Src> value) {
    {*src.begin() = value};
    {src.erase(std::remove_if(src.begin(), src.end(), func), src.end())};
};

template <typename Src>
concept Erasable = SingleSocketedStdContainer<Src> && requires(Src src, InnerType_T<Src> value) {
    {src.erase(value)};
    {src.erase(src.begin())};
};
// clang-format on
} // namespace cefal::detail
