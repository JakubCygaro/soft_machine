#pragma once

#include <concepts>
#include <variant>

struct Unit { };
inline static Unit unit()
{
    static Unit u;
    return u;
}

template <std::movable Err, std::movable Ok>
using result_t = std::variant<Err, Ok>;

template <std::movable Err>
using unit_result_t = std::variant<Err, Unit>;

namespace result {
    template <std::movable Err, std::movable Ok>
    inline static bool isok(const result_t<Err, Ok>& r) noexcept {
        return std::holds_alternative<Ok>(r);
    }
    template <std::movable Err, std::movable Ok>
    inline static bool iserr(const result_t<Err, Ok>& r) noexcept {
        return std::holds_alternative<Err>(r);
    }
    template <std::movable Err, std::movable Ok>
    inline static Ok unwrap(result_t<Err, Ok>&& r) {
        return std::get<Ok>(r);
    }
    template <std::movable Err, std::movable Ok>
    inline static Err unwrap_err(result_t<Err, Ok>&& r) {
        return std::get<Err>(r);
    }
}
