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
#include "cefal/monoid.h"

#include <type_traits>

namespace cefal {
namespace helpers {
template <cefal::detail::SingleSocketedStdContainer Src>
struct SingletonFrom<Src> {
    using value_type = typename Src::value_type;
    value_type value;
};
} // namespace helpers

namespace instances {
namespace detail {
template <cefal::detail::Reservable C>
void reserveContainer(C& c, size_t size) {
    c.reserve(size);
}
template <typename C>
void reserveContainer(C& c, size_t size) {
}
} // namespace detail

template <cefal::detail::VectorLikeContainer Src>
struct Monoid<Src> {
    static Src empty() { return Src(); }

    static Src append(const Src& left, const Src& right) {
        if (!left.size())
            return right;
        if (!right.size())
            return left;
        Src result;
        detail::reserveContainer(result, left.size() + right.size());
        result.insert(result.end(), left.begin(), left.end());
        result.insert(result.end(), right.begin(), right.end());
        return result;
    }

    static Src append(Src&& left, const Src& right) {
        if (!left.size())
            return right;
        if (!right.size())
            return std::move(left);
        detail::reserveContainer(left, left.size() + right.size());
        left.insert(left.end(), right.begin(), right.end());
        return std::move(left);
    }

    static Src append(const Src& left, Src&& right) {
        if (!left.size())
            return std::move(right);
        if (!right.size())
            return left;
        detail::reserveContainer(right, left.size() + right.size());
        right.insert(right.begin(), left.begin(), left.end());
        return std::move(right);
    }

    static Src append(Src&& left, Src&& right) {
        if (!left.size())
            return std::move(right);
        if (!right.size())
            return std::move(left);
        if (left.size() < right.size()) {
            detail::reserveContainer(right, left.size() + right.size());
            right.insert(right.begin(), left.begin(), left.end());
            return std::move(right);
        } else {
            detail::reserveContainer(left, left.size() + right.size());
            left.insert(left.end(), right.begin(), right.end());
            return std::move(left);
        }
    }

    template <typename T>
    static Src append(T&& left, helpers::SingletonFrom<Src>&& right) {
        static_assert(std::is_same_v<std::remove_cvref_t<T>, Src>, "Argument type should be the same as monoid");
        Src result = std::forward<T>(left);
        result.push_back(std::move(right.value));
        return result;
    }
};

template <cefal::detail::SetLikeContainer Src>
struct Monoid<Src> {
    static Src empty() { return Src(); }

    template <typename T1, typename T2>
    static Src append(T1&& left, T2&& right) {
        static_assert(std::is_same_v<std::remove_cvref_t<T1>, Src>, "Argument type should be the same as monoid");
        static_assert(std::is_same_v<std::remove_cvref_t<T2>, Src>, "Argument type should be the same as monoid");
        Src result = std::forward<T1>(left);
        result.merge(std::forward<T2>(right));
        return result;
    }

    template <typename T>
    static Src append(T&& left, helpers::SingletonFrom<Src>&& right) {
        static_assert(std::is_same_v<std::remove_cvref_t<T>, Src>, "Argument type should be the same as monoid");
        Src result = std::forward<T>(left);
        result.insert(std::move(right.value));
        return result;
    }
};
} // namespace instances
} // namespace cefal
