#include "game/Styling.hpp"
#include "s7.h"
#include "scheme/Util.hpp"
#include <variant>
namespace scheme {
std::optional<game::StylingParameter>
obj_to_styling_param(::s7_scheme* scm, ::s7_pointer obj)
{
    if (::s7_is_integer(obj)) {
        return int(
            ::s7_integer(obj));
    } else if (::s7_is_real(obj)) {
        return double(
            ::s7_real(obj));
    } else if (::s7_is_string(obj)) {
        return std::string(
            ::s7_string(obj));
    } else if (::s7_is_boolean(obj)) {
        return bool(
            ::s7_boolean(scm, obj));
    }
    return std::nullopt;
}
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
::s7_pointer
styling_param_to_obj(::s7_scheme* scm,
    const game::StylingParameter& param)
{
    return std::visit(
        overloaded {
            [=](const int& i) { return ::s7_make_integer(scm, i); },
            [=](const float& f) { return ::s7_make_real(scm, f); },
            [=](const double& d) { return ::s7_make_real(scm, d); },
            [=](const bool& b) { return ::s7_make_boolean(scm, b); },
            [=](const std::string& s) { return ::s7_make_string(scm, s.c_str()); },
            [=](const auto&) { return ::s7_nil(scm); } },
        param);
}
}
