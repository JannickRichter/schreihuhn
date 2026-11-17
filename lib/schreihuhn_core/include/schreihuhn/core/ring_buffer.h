#pragma once

#include <array>
#include <cstddef>

namespace schreihuhn::core {

template <typename T, std::size_t Capacity>
class StaticRingBuffer {
public:
    static_assert(Capacity > 0, "StaticRingBuffer capacity must be > 0");

    constexpr void clear() {
        head_ = 0;
        count_ = 0;
    }

    constexpr bool pushBack(const T& item) {
        if (count_ >= Capacity) {
            return false;
        }
        const std::size_t tail = (head_ + count_) % Capacity;
        data_[tail] = item;
        ++count_;
        return true;
    }

    constexpr void popFront() {
        if (count_ == 0) {
            return;
        }
        head_ = (head_ + 1) % Capacity;
        --count_;
    }

    constexpr std::size_t size() const { return count_; }
    constexpr std::size_t capacity() const { return Capacity; }
    constexpr bool empty() const { return count_ == 0; }
    constexpr bool full() const { return count_ >= Capacity; }

    constexpr T& operator[](std::size_t idx) {
        return data_[(head_ + idx) % Capacity];
    }

    constexpr const T& operator[](std::size_t idx) const {
        return data_[(head_ + idx) % Capacity];
    }

private:
    std::array<T, Capacity> data_{};
    std::size_t head_ = 0;
    std::size_t count_ = 0;
};

} // namespace schreihuhn::core
