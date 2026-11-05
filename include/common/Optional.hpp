#pragma once
#include <optional>
namespace common {
template <class T, class Func>
constexpr inline void visit_opt(std::optional<T>& opt, Func fn)
{
    using unit_t = struct { };
    using bullshit_t = std::optional<unit_t>;
    opt.and_then([&fn](auto& o) -> bullshit_t {
        fn(o);
        return { };
    });
}

}
