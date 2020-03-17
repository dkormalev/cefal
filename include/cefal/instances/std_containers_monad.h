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
#include "cefal/detail/type_traits.h"
#include "cefal/monad.h"

#include <algorithm>
#include <type_traits>

namespace cefal::instances {
template <detail::StdContainer Src>
struct Monad<Src> {
private:
    using T = detail::InnerType_T<Src>;

public:
    template <typename Func, detail::SingleSocketedStdContainer RawResult = std::invoke_result_t<Func, T>>
    static auto flatMap(const Src& src, Func&& func) requires detail::VectorLikeContainer<Src> {
        using Result = detail::WithInnerType_T<Src, detail::InnerType_T<RawResult>>;
        Result result;
        for (auto&& t : src) {
            RawResult mapped = func(t);
            for (auto&& x : mapped)
                result.push_back(std::move(x));
        }
        return result;
    }

    template <typename Func, detail::SingleSocketedStdContainer RawResult = std::invoke_result_t<Func, T>>
    static auto flatMap(Src&& src, Func&& func) requires detail::VectorLikeContainer<Src> {
        using Result = detail::WithInnerType_T<Src, detail::InnerType_T<RawResult>>;
        Result result;
        for (auto&& t : src) {
            RawResult mapped = func(std::move(t));
            for (auto&& x : mapped)
                result.push_back(std::move(x));
        }
        return result;
    }

    template <typename Func, detail::SingleSocketedStdContainer RawResult = std::invoke_result_t<Func, T>>
    static auto flatMap(const Src& src, Func&& func) requires detail::SetLikeContainer<Src> {
        using Result = detail::WithInnerType_T<Src, detail::InnerType_T<RawResult>>;
        Result result;
        for (auto&& t : src) {
            RawResult mapped = func(t);
            if constexpr (std::is_same_v<Result, RawResult>) {
                result.merge(std::move(mapped));
            } else {
                for (auto&& x : mapped)
                    result.insert(std::move(x));
            }
        }
        return result;
    }

    template <typename Func, detail::SingleSocketedStdContainer RawResult = std::invoke_result_t<Func, T>>
    static auto flatMap(Src&& src, Func&& func) requires detail::SetLikeContainer<Src> {
        using Result = detail::WithInnerType_T<Src, detail::InnerType_T<RawResult>>;
        Result result;
        for (auto&& t : src) {
            RawResult mapped = func(std::move(t));
            if constexpr (std::is_same_v<Result, RawResult>) {
                result.merge(std::move(mapped));
            } else {
                for (auto&& x : mapped)
                    result.insert(std::move(x));
            }
        }
        return result;
    }
};
} // namespace cefal::instances
