#include "common/reflect/Enum.hpp"
#include "game/Styling.hpp"
#include "s7.h"
#include "scheme/Util.hpp"
#include <raylib.h>
namespace {
extern "C" {
static ::s7_pointer style_set_param(::s7_scheme* scm, ::s7_pointer args)
{
    if (!::s7_is_string(::s7_car(args))) {
        return (::s7_wrong_type_arg_error(scm,
            "set!-param",
            1,
            args,
            "(cons 'key 'value)"));
    }
    const auto key = std::string(::s7_string(::s7_car(args)));
    const auto value = scheme::obj_to_styling_param(scm, ::s7_cdr(args));
    if (!value) {
        return (::s7_wrong_type_arg_error(scm,
            "set!-param",
            1,
            args,
            "(cons 'key 'value)"));
    }
    game::Style::instance_set_param(key, *value);
    return (::s7_nil(scm));
}
static ::s7_pointer style_get_param(::s7_scheme* scm, ::s7_pointer args)
{
    std::optional<std::string> target;
    if (!::s7_is_string(::s7_car(args))) {
        return (::s7_wrong_type_arg_error(scm,
            "get-param",
            1,
            args,
            "(cons 'param 'optional-name)"));
    }
    if (::s7_is_string(::s7_cdr(args))) {
        target = std::string(::s7_string(::s7_cdr(args)));
    } else if (!::s7_is_null(scm, ::s7_cdr(args))) {
        return (::s7_wrong_type_arg_error(scm,
            "get-param",
            1,
            args,
            "(cons 'param 'optional-name)"));
    }
    const auto key = std::string(::s7_string(::s7_car(args)));
    if(auto param = game::Style::instance_get_param(key, target); param){
        return (scheme::styling_param_to_obj(scm, *param));
    } else {
        return (::s7_nil(scm));
    }

}
static ::s7_pointer style_set_font(::s7_scheme* scm, ::s7_pointer args)
{
    if (!::s7_is_string(::s7_car(args))) {
        return (::s7_wrong_type_arg_error(scm,
            "set!-font",
            1,
            args,
            "string"));
    }
    game::Style::instance_set_font(::s7_string(::s7_car(args)));
    return (::s7_nil(scm));
}
static ::s7_pointer style_get_font(::s7_scheme* scm, ::s7_pointer args)
{
    std::optional<std::string> param;
    if (::s7_is_string(::s7_car(args))) {
        param = std::string(::s7_string(::s7_car(args)));
    }
    const auto f = game::Style::instance_get_font(param);
    return (::s7_make_string(scm, f.c_str()));
}
static ::s7_pointer style_set_font_size(::s7_scheme* scm, ::s7_pointer args)
{
    if (!::s7_is_real(::s7_car(args))) {
        return (::s7_wrong_type_arg_error(scm,
            "set!-font-size",
            1,
            args,
            "real"));
    }
    game::Style::instance_set_font_size(::s7_real(::s7_car(args)));
    return (::s7_nil(scm));
}
static ::s7_pointer style_get_font_size(::s7_scheme* scm, ::s7_pointer args)
{
    std::optional<std::string> param;
    if (::s7_is_string(::s7_car(args))) {
        param = std::string(::s7_string(::s7_car(args)));
    }
    const auto e = game::Style::instance_get_font_size(param);
    return (::s7_make_boolean(scm, e));
}
static ::s7_pointer style_connection(::s7_scheme* scm, ::s7_pointer args)
{
    std::optional<std::string> param;
    if (::s7_is_string(::s7_car(args))) {
        param = std::string(::s7_string(::s7_car(args)));
    }
    const auto e = game::Style::instance_is_connection(param);
    return (::s7_make_boolean(scm, e));
}
static ::s7_pointer style_component(::s7_scheme* scm, ::s7_pointer args)
{

    std::optional<std::string> param;
    if (::s7_is_string(::s7_car(args))) {
        param = std::string(::s7_string(::s7_car(args)));
    }
    const auto e = game::Style::instance_is_component(param);
    return (::s7_make_boolean(scm, e));
}
// ex. (exists? "CPU")
static ::s7_pointer style_exists(::s7_scheme* scm, ::s7_pointer args)
{
    if (!::s7_is_string(::s7_car(args))) {
        return (::s7_wrong_type_arg_error(scm,
            "exists?",
            1,
            args,
            "string"));
    }
    const auto e = game::Style::instance_exists(std::string(
        ::s7_string(::s7_car(args))));
    return (::s7_make_boolean(scm, e));
}
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
        "(cons 'x 'y)"));
}
static ::s7_pointer style_get_abs_pos(::s7_scheme* scm, ::s7_pointer args)
{
    std::optional<std::string> param;
    // ex. (get-abs-pos "CPU")
    if (::s7_is_string(::s7_car(args))) {
        param = std::string(::s7_string(args));
    } else if (!::s7_is_null(scm, args)) {
        return (::s7_wrong_type_arg_error(scm,
            "get-abs-pos",
            1,
            args,
            "either string or nil"));
    }
    const auto p = game::Style::instance_get_abs_pos(param);
    return (::s7_cons(scm,
        ::s7_make_real(scm, p.x),
        ::s7_make_real(scm, p.y)));
}
// called like (set!-rel-pos "name" 'left 400)
static ::s7_pointer style_set_rel_pos(::s7_scheme* scm, ::s7_pointer args)
{
    game::RelativePos rp;
    if (!::s7_is_string(::s7_car(args))) {
        return (::s7_wrong_type_arg_error(scm,
            "set!-rel-pos",
            1,
            ::s7_car(args),
            "string"));
    } else {
        rp.rel_to = ::s7_string(::s7_car(args));
    }
    if (!::s7_is_symbol(::s7_cadr(args))) {
        return (::s7_wrong_type_arg_error(scm,
            "set!-rel-pos",
            1,
            ::s7_cadr(args),
            "symbol"));
    } else {
        const auto syn = std::string(s7_symbol_name(::s7_cadr(args)));
        using enum game::RelativePos::Side;
        auto side = common::reflect::string_to_enum<game::RelativePos::Side,
            common::reflect::MatchCase::IgnoreCase>(syn);
        if (side) {
            rp.side = *side;
        } else {
            return (::s7_wrong_type_arg_error(scm,
                "set!-rel-pos",
                1,
                ::s7_cadr(args),
                "(cons 'x 'y)"));
        }
    }
    if (!::s7_is_real(::s7_caddr(args))) {
        rp.distance = 0;
    } else {
        rp.distance = static_cast<float>(::s7_real(::s7_caddr(args)));
    }
    game::Style::instance_set_rel_pos(rp);
    return (::s7_nil(scm));
}
// ex. (get-rel-pos)
static ::s7_pointer style_get_rel_pos(::s7_scheme* scm, ::s7_pointer)
{
    const auto p = game::Style::instance_get_rel_pos();
    auto ret = ::s7_make_list(scm, 3, ::s7_nil(scm));
    ::s7_list_set(scm, ret, 0, ::s7_make_string(scm, p->rel_to.c_str()));
    ::s7_list_set(scm,
        ret,
        1,
        ::s7_make_symbol(scm, common::reflect::enum_to_string(p->side).c_str()));
    ::s7_list_set(scm, ret, 2, ::s7_make_real(scm, p->distance));
    return (ret);
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
        "get-abs-pos",
        ::style_get_abs_pos,
        0,
        1,
        false,
        "get absolute position of this or some other element");
    ::s7_define_function(scm,
        "set!-rel-pos",
        ::style_set_rel_pos,
        2,
        1,
        false,
        "set relative position of this");
    ::s7_define_function(scm,
        "get-rel-pos",
        ::style_get_rel_pos,
        0,
        0,
        false,
        "get relative position of this");
    ::s7_define_function(scm,
        "exists?",
        ::style_exists,
        1,
        0,
        false,
        "check if a graph element exists");
    ::s7_define_function(scm,
        "component?",
        ::style_component,
        0,
        1,
        false,
        "check if a graph element is a component");
    ::s7_define_function(scm,
        "connection?",
        ::style_connection,
        0,
        1,
        false,
        "check if a graph element is a connection");
    ::s7_define_function(scm,
        "set!-param",
        ::style_set_param,
        2,
        0,
        false,
        "set custom styling parameter for this element");
    ::s7_define_function(scm,
        "get-param",
        ::style_get_param,
        1,
        1,
        false,
        "get custom styling parameter for this or some other element");
    ::s7_define_function(scm,
        "set!-font",
        ::style_set_font,
        1,
        0,
        false,
        "set font for this element");
    ::s7_define_function(scm,
        "get-font",
        ::style_get_font,
        1,
        1,
        false,
        "get font for this or some other element");
    ::s7_define_function(scm,
        "set!-font-size",
        ::style_set_font_size,
        1,
        0,
        false,
        "set font size for this element");
    ::s7_define_function(scm,
        "get-font-size",
        ::style_get_font_size,
        1,
        1,
        false,
        "get font size for this or some other element");
}
}
