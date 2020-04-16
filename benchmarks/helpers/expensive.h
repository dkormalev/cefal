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

#include <algorithm>
#include <unordered_set>

template <typename T>
struct Expensive {
    Expensive(T x = T()) : value(std::move(x)), payload(new char[102400]) {}
    Expensive(T x, char* payload) : value(std::move(x)), payload(payload) {}
    Expensive(const Expensive& other) {
        payload = new char[102400];
        value = other.value;
    }
    Expensive(Expensive&& other) {
        payload = other.payload;
        value = std::move(other.value);
        other.payload = nullptr;
    }
    Expensive& operator=(const Expensive& other) {
        delete[] payload;
        payload = new char[102400];
        value = other.value;
        return *this;
    }
    Expensive& operator=(Expensive&& other) {
        std::swap(payload, other.payload);
        value = std::move(other.value);
        return *this;
    }
    ~Expensive() { delete[] payload; }

    operator T() const { return value; }

    template <typename U>
    bool operator%(U&& other) const {
        return value % other;
    }

    template <typename U>
    Expensive<std::remove_cvref_t<U>> operator+(U&& other) const& {
        return Expensive<std::remove_cvref_t<U>>(value + other);
    }

    template <typename U>
    Expensive<std::remove_cvref_t<U>> operator+(U&& other) && {
        auto result = Expensive<std::remove_cvref_t<U>>(value + other, payload);
        payload = nullptr;
        return result;
    }

    Expensive& operator+=(T other) {
        value += other;
        return *this;
    }

    auto operator<=>(const Expensive<T>& other) const { return value <=> other.value; }
    bool operator==(const Expensive<T>& other) const { return std::abs(value - other.value) < 0.0001; }

    T value = T();
    char* payload = nullptr;
};

namespace std {
template <typename T>
struct hash<Expensive<T>> {
    size_t operator()(const Expensive<T>& x) const { return std::hash<T>()(x.value); }
};
} // namespace std
