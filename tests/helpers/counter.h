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

#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>

class Counter {
public:
    Counter() noexcept { _created++; }
    Counter(const Counter&) noexcept { _copied++; }
    Counter(Counter&&) noexcept { _moved++; }
    Counter& operator=(const Counter&) noexcept {
        _copied++;
        return *this;
    }
    Counter& operator=(Counter&&) noexcept {
        _moved++;
        return *this;
    }
    ~Counter() noexcept { _deleted++; }

    auto operator<=>(const Counter&) const = default;

    static uint64_t created() { return _created; }
    static uint64_t copied() { return _copied; }
    static uint64_t moved() { return _moved; }
    static uint64_t deleted() { return _deleted; }
    static uint64_t customCount() {
        std::unique_lock<std::mutex> lock(customMutex);
        return _custom.size();
    }
    static uint64_t custom(const std::string& name) {
        std::unique_lock<std::mutex> lock(customMutex);
        return _custom[name];
    }

    static void addCustom(const std::string& name) {
        std::unique_lock<std::mutex> lock(customMutex);
        ++_custom[name];
    }

    static void resetCreated() { _created = 0; }
    static void resetCopied() { _copied = 0; }
    static void resetMoved() { _moved = 0; }
    static void resetDeleted() { _deleted = 0; }
    static void resetCustom(const std::string& name) {
        std::unique_lock<std::mutex> lock(customMutex);
        _custom[name] = 0;
    }
    static void reset() {
        resetCreated();
        resetCopied();
        resetMoved();
        resetDeleted();
        std::unique_lock<std::mutex> lock(customMutex);
        _custom.clear();
    }

private:
    static std::atomic_uint64_t _created;
    static std::atomic_uint64_t _copied;
    static std::atomic_uint64_t _moved;
    static std::atomic_uint64_t _deleted;

    static std::mutex customMutex;
    static std::unordered_map<std::string, uint64_t> _custom;
};

struct CountedValue : public Counter {
    CountedValue() : Counter() {}
    CountedValue(int value) : Counter(), value(value) {}
    CountedValue(const CountedValue&) noexcept = default;
    CountedValue(CountedValue&&) noexcept = default;
    CountedValue& operator=(const CountedValue&) noexcept = default;
    CountedValue& operator=(CountedValue&&) noexcept = default;
    int value = 0;
    int64_t u1 = 0;
    int64_t u2 = 0;
    auto operator<=>(const CountedValue&) const = default;
};

inline std::ostream& operator<<(std::ostream& os, const CountedValue& value) {
    os << std::to_string(value.value);
    return os;
}
inline std::ostream& operator<<(std::ostream& os, const std::pair<int, CountedValue>& value) {
    os << std::to_string(value.first) << std::string(":") << value.second;
    return os;
}
inline std::ostream& operator<<(std::ostream& os, const std::pair<CountedValue, CountedValue>& value) {
    os << value.first << std::string(":") << value.second;
    return os;
}

namespace std {
template <>
struct hash<CountedValue> {
    std::size_t operator()(const CountedValue& x) const noexcept { return std::hash<int>{}(x.value); }
};
template <>
struct hash<pair<int, CountedValue>> {
    std::size_t operator()(const pair<int, CountedValue>& x) const noexcept {
        return std::hash<int>{}(x.first) ^ (std::hash<CountedValue>{}(x.second) << 1);
    }
};
template <>
struct hash<pair<CountedValue, CountedValue>> {
    std::size_t operator()(const pair<CountedValue, CountedValue>& x) const noexcept {
        return std::hash<CountedValue>{}(x.first) ^ (std::hash<CountedValue>{}(x.second) << 1);
    }
};
} // namespace std
