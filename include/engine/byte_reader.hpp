#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace engine {

template <typename T>
T read_be(std::span<const std::uint8_t> bytes, std::size_t offset) {
    T result = 0;
    for (std::size_t i = 0; i < sizeof(T); ++i) {
        result = (result << 8) | bytes[offset + i];
    }
    return result;
}

} // namespace engine
