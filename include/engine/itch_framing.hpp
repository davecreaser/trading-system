#pragma once

#include <cstdint>
#include <istream>
#include <optional>
#include <vector>

namespace engine {

std::optional<std::vector<std::uint8_t>> read_next_message(std::istream& stream);

} // namespace engine