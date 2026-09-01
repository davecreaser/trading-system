#include "engine/itch_framing.hpp"
#include "engine/byte_reader.hpp"

namespace engine {

std::optional<std::vector<std::uint8_t>> read_next_message(std::istream& stream) {
    std::uint8_t buffer[2];
    if (!stream.read(reinterpret_cast<char*>(buffer), 2)) {
        return std::nullopt;
    };
    int message_length = read_be<std::uint16_t>(buffer, 0);

    std::vector<std::uint8_t> message(message_length);
    stream.read(reinterpret_cast<char*>(message.data()), message_length);
    return message;
};

} // namespace engine