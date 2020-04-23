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

#include <concepts>

namespace cefal::detail {
// clang-format off
template <template <typename ...> typename T>
concept ZeroSockets = requires {typename T<>;};
template <template <typename ...> typename T>
concept OneSocket = (!ZeroSockets<T>) && requires {typename T<int>;};
template <template <typename ...> typename T>
concept TwoSockets = (!ZeroSockets<T>) && (!OneSocket<T>) && requires {typename T<int, double>;};
template <template <typename ...> typename T>
concept ThreeSockets = (!ZeroSockets<T>) && (!OneSocket<T>) && (!TwoSockets<T>) && requires {typename T<int, double, float>;};
template <template <typename ...> typename T>
concept FourSockets = (!ZeroSockets<T>) && (!OneSocket<T>) && (!TwoSockets<T>) && (!ThreeSockets<T>)
                      && requires {typename T<int, double, float, bool>;};
template <template <typename ...> typename T>
concept FiveSockets = (!ZeroSockets<T>) && (!OneSocket<T>) && (!TwoSockets<T>) && (!ThreeSockets<T>) && (!FourSockets<T>)
                      && requires {typename T<int, double, float, bool, char>;};
// clang-format on

template <template <typename...> typename T>
struct Instantiator {};

template <template <typename...> typename T>
requires ZeroSockets<T> struct Instantiator<T> {
    using type = T<>;
};

template <template <typename...> typename T>
requires OneSocket<T> struct Instantiator<T> {
    using type = T<int>;
};

template <template <typename...> typename T>
requires TwoSockets<T> struct Instantiator<T> {
    using type = T<int, double>;
};

template <template <typename...> typename T>
requires ThreeSockets<T> struct Instantiator<T> {
    using type = T<int, double, float>;
};

template <template <typename...> typename T>
requires FourSockets<T> struct Instantiator<T> {
    using type = T<int, double, float, bool>;
};

template <template <typename...> typename T>
requires FiveSockets<T> struct Instantiator<T> {
    using type = T<int, double, float, bool, char>;
};

template <template <typename...> typename T>
using Instantiator_T = Instantiator<T>::type;
} // namespace cefal::detail
