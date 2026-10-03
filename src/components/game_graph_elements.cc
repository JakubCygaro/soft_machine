#include "common/String.hpp"
#include "components/GameGraphElements.hpp"
namespace components {

Result<std::runtime_error, std::any>
parse_value_msg_kind(const ValueMsgKind& kind, const std::string& text)
{
    using enum components::ValueMsgKind;
    std::any msg;
    switch (kind) {
    case String: {
        msg = common::trim(text);
    } break;
    case Number: {
        auto trimmed = common::trim(text);
        int out;
        auto res = std::from_chars(
            trimmed.data(),
            trimmed.data() + trimmed.size(),
            out,
            10);
        if (res.ec == std::errc::invalid_argument) {
            return { std::runtime_error(
                std::format("Failed to parse clock value node as a number")) };
        }
        msg = out;
    } break;
    }
    return { msg };
}
}
