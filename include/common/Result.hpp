#pragma once

#include <concepts>
#include <functional>
#include <variant>

struct Unit { };
inline static Unit unit()
{
    static Unit u;
    return u;
}

template <std::movable Err, std::movable Ok>
struct Result : public std::variant<Err, Ok> {
    using ok_t = Ok;
    using err_t = Err;
    inline static Result ok(Ok&& ok)
    {
        return Result(ok);
    }
    inline static Result ok(Ok& ok)
    {
        return Result(ok);
    }
    inline static Result err(Err&& err)
    {
        return Result(err);
    }
    inline static Result err(Err& err)
    {
        return Result(err);
    }
    inline bool isok() const noexcept
    {
        return std::holds_alternative<Ok>(*this);
    }
    inline bool iserr() const noexcept
    {
        return std::holds_alternative<Err>(*this);
    }
    inline Ok unwrap()
    {
        return std::get<Ok>(std::move(*this));
    }
    inline Err unwrap_err()
    {
        return std::get<Err>(*this);
    }
    template <std::movable NewOk>
    inline Result<Err, NewOk> map_ok(std::function<NewOk(Ok&&)> map)
    {
        Result<Err, NewOk>::ok(map(unwrap()));
    }
    template <std::movable NewErr>
    inline Result<NewErr, Ok> map_err(std::function<NewErr(Err&&)> map)
    {
        Result<NewErr, Ok>::err(map(unwrap_err()));
    }
};

template <std::movable Err>
using unit_result_t = Result<Err, Unit>;
