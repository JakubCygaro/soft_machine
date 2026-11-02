#include "game/Styling.hpp"
#include "s7.h"
#include <raylib.h>
#ifndef CLANGD_SKIP
#include <meta>
#endif
namespace {
extern "C" {
static ::s7_pointer style_set_abs_pos(::s7_scheme* scm, ::s7_pointer args)
{
    if (::s7_is_real(::s7_car(args))
        && ::s7_is_real(::s7_car(::s7_cdr(args)))) {
        game::Style::instance_set_abs_pos({
            .x = static_cast<float>(::s7_real(::s7_car(args))),
            .y = static_cast<float>(::s7_real(::s7_cdr(args))),
        });
        return (::s7_nil(scm));
    }
    return (::s7_wrong_type_arg_error(scm,
        "set!-abs-pos",
        1,
        args,
        "expected position"));
}
static ::s7_pointer style_get_abs_pos(::s7_scheme* scm, ::s7_pointer)
{
    const auto p = game::Style::instance_get_abs_pos();
    return (::s7_cons(scm,
        ::s7_make_real(scm, p.x),
        ::s7_make_real(scm, p.y)));
}
}
}
namespace game {

void StyleSchemeWrap::init(::s7_scheme* scm)
{
    ::s7_define_function(scm,
        "set!-abs-pos",
        ::style_set_abs_pos,
        2,
        0,
        false,
        "set absolute position");
    ::s7_define_function(scm,
        "get!-abs-pos",
        ::style_get_abs_pos,
        0,
        0,
        false,
        "get absolute position");
}
}
