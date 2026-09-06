#pragma once

#include <cstdint>
#include <istream>
#include <optional>
#include <string_view>

namespace engine {

std::optional<std::uint16_t> resolve_stock_locate(std::istream& stream, std::string_view symbol);

}  // namespace engine