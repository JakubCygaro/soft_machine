#pragma once
#include "common/Result.hpp"
#include "game/Xml.hpp"
#include <concepts>
#include <format>
#ifndef CLANGD_SKIP
#include <meta>
#endif
#include <pugixml.hpp>
#include <stdexcept>

namespace game::xml {
template<typename T, typename From>
concept ParsableFrom = requires(T t, From from){
    {  T::parse(from) } -> std::same_as<Result<std::runtime_error, T>>;
};


template <std::default_initializable T>
Result<std::runtime_error, T> unmarshall_attributes(const pugi::xml_node& n)
#ifndef CLANGD_SKIP
{
    T ret { };
    constexpr auto ctx = std::meta::access_context::current();
    constexpr auto info = ^^T;
    template for (constexpr auto member : std::define_static_array(
                      std::meta::nonstatic_data_members_of(info, ctx)))
    {
        using member_type = typename[:type_of(member):];
        constexpr std::string_view member_ident = std::meta::identifier_of(member);
        auto attr = n.attribute(member_ident);
        if (attr.empty()) {
            return { std::runtime_error(
                std::format(
                    "Attribute '{}' not found", member_ident)) };
        }
        if constexpr (std::is_same_v<member_type, std::string>) {
            ret.[:member:] = attr.as_string();
        } else if constexpr (std::is_same_v<member_type, unsigned int>) {
            ret.[:member:] = attr.as_uint();
        } else if constexpr (std::is_same_v<member_type, int>) {
            ret.[:member:] = attr.as_int();
        } else if constexpr (std::is_same_v<member_type, double>) {
            ret.[:member:] = attr.as_double();
        } else if constexpr (std::is_same_v<member_type, float>) {
            ret.[:member:] = attr.as_float();
        } else if constexpr (ParsableFrom<member_type, const char*>) {
            const char* str = attr.as_string();
            auto parsed = member_type::parse(str);
            if (parsed.iserr()){
                return { parsed.unwrap_err() };
            }
            ret.[:member:] = parsed.unwrap();
        } else {
            static_assert(false, "Unsupported attribute struct member type");
        }
    }
    return { ret };
}
#else
    ;
#endif
}
