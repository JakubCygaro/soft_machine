#pragma once
#include <variant>

namespace machine {

struct Unit { };

template <std::movable Err, std::movable Ok>
using result_t = std::variant<Err, Ok>;

}
