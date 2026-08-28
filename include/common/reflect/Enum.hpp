#pragma once

#include <optional>
#include <string>
#ifndef CLANGD_SKIP
#include <meta>
#endif
namespace common::reflect {

template <typename E>
#ifndef CLANGD_SKIP
    requires(std::meta::is_enum_type(^^E))
#endif
constexpr std::string
enum_to_string(E e)
#ifndef CLANGD_SKIP
{
    constexpr static auto enumerators = std::define_static_array(enumerators_of(^^E));
    template for (constexpr auto enumerator : enumerators)
    {
        constexpr auto iden = std::meta::identifier_of(enumerator);
        constexpr E val = [:enumerator:];
        if (e == val) {
            return std::string(iden);
        }
    }
    return "";
}
#else
{
    (void)e;
    return "";
}
#endif

enum class MatchCase {
    IgnoreCase,
    ExactCase
};

template <typename E, MatchCase M = MatchCase::ExactCase>
#ifndef CLANGD_SKIP
    requires(std::meta::is_enum_type(^^E))
#endif
constexpr std::optional<E> string_to_enum(const std::string_view sv)
#ifndef CLANGD_SKIP
{
    constexpr static auto enumerators = std::define_static_array(enumerators_of(^^E));
    template for (constexpr auto enumerator : enumerators)
    {
        constexpr auto iden = std::meta::identifier_of(enumerator);
        if (std::ranges::equal(sv, iden, [](const auto& a, const auto& b) {
                if constexpr (M == MatchCase::IgnoreCase)
                    return std::tolower(a) == std::tolower(b);
                else
                    return a == b;
            })) {
            constexpr E val = [:enumerator:];
            return E(val);
        }
    }
    return std::nullopt;
}
#else
{
    (void)sv;
}
#endif

}
