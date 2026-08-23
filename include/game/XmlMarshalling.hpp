#pragma once
#include "common/Result.hpp"
#include "game/Xml.hpp"
#include <algorithm>
#include <concepts>
#include <format>
#include <ranges>
#ifndef CLANGD_SKIP
#include <meta>
#endif
#include <pugixml.hpp>
#include <stdexcept>

namespace game::xml {
template <typename T, typename From>
concept ParsableFromXml = requires(T t, From from) {
    { T::parse_xml(from) } -> std::same_as<Result<std::runtime_error, T>>;
};
template <typename Optional>
struct strip_optional {
    using type = Optional;
};
template <typename T>
struct strip_optional<std::optional<T>> {
    using type = T;
};

template <typename T>
struct Attribute {
    T val;
    inline Attribute& operator=(T&& v) noexcept
    {
        val = v;
        return *this;
    }
    inline Attribute& operator=(T& v) noexcept
    {
        val = v;
        return *this;
    }
    inline operator T&() { return val; }
    inline operator const T&() const { return val; }
    inline T* operator->() { return &val; }
    inline const T* operator->() const { return &val; }
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
        constexpr auto is_optional = std::is_assignable_v<
            typename[:type_of(member):], std::nullopt_t>;
        using member_type = typename strip_optional<typename[:type_of(member):]>::type;
        constexpr std::string_view member_ident = std::meta::identifier_of(member);
        auto attr = n.attribute(member_ident);
        if (attr.empty()) {
            if constexpr (is_optional) {
                ret.[:member:] = std::nullopt;
                continue;
            }
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
        } else if constexpr (ParsableFromXml<member_type, const char*>) {
            const char* str = attr.as_string();
            auto parsed = member_type::parse_xml(str);
            if (parsed.iserr()) {
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

template <std::default_initializable T>
Result<std::runtime_error, T> unmarshall_node(pugi::xml_node& n)
#ifndef CLANGD_SKIP
{
    constexpr auto ctx = std::meta::access_context::current();
    constexpr auto info = ^^T;
    if constexpr (ParsableFromXml<T, pugi::xml_text>) {
        auto str = n.text();
        return T::parse_xml(str);
    }
    T ret { };
    template for (constexpr auto member : std::define_static_array(
                      std::meta::nonstatic_data_members_of(info, ctx)))
    {
        constexpr auto is_optional = std::is_assignable_v<
            typename[:type_of(member):], std::nullopt_t>;
        constexpr auto member_type = std::meta::type_of(member);
        using stripped_member_type =
            typename strip_optional<typename[:type_of(member):]>::type;
        constexpr std::string_view member_ident = std::meta::identifier_of(member);
        if constexpr (std::meta::has_template_arguments(member_type)
            && (std::meta::template_of(member_type) == ^^Attribute)) {
            constexpr auto arg0 = std::meta::template_arguments_of(member_type)[0];
            using arg0_t = typename[:arg0:];
            if (auto unm = unmarshall_attributes<arg0_t>(n); unm.iserr()) {
                return { unm.unwrap_err() };
            } else {
                ret.[:member:] = unm.unwrap();
            }
        } else if constexpr (member_ident == "text") {
            ret.[:member:] = n.text().get();
        } else {
            auto ch = n.find_child([=](auto child) {
                return child.name() == member_ident;
            });
            if (!ch && !is_optional) {
                return {
                    std::runtime_error(
                        std::format("member {} not present in xml", member_ident))
                };
            } else if (auto unm = unmarshall_node<stripped_member_type>(ch); unm.isok()) {
                ret.[:member:] = unm.unwrap();
            } else {
                return { unm.unwrap_err() };
            }
        }
    }
    return { ret };
}
#else
    ;
#endif
}
