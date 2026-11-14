#pragma once

#include "game/Styling.hpp"
#include "s7.h"
namespace scheme {
std::optional<game::StylingParameter> obj_to_styling_param(::s7_scheme*, ::s7_pointer);
::s7_pointer styling_param_to_obj(::s7_scheme*, const game::StylingParameter&);
}
