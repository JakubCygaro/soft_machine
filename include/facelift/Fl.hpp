#pragma once

#include <array>
#ifndef CLANGD_SKIP
#define ANNOTATE(VAL) \
    [[= VAL]]
#else
#define ANNOTATE(VAL)
#endif
namespace fl {
inline static constexpr std::size_t string_param_size = 128;
using string_param_t = std::array<char, string_param_size>;

// annotation used for selecting a component to be included in the editor
// component spawner
inline static constexpr struct {
} component { };
// annotation used for selecting a constructor of a component used to create it
// in facelift
inline static constexpr struct {
} use_ctor { };
// annotation used for ignoring constructor parameters
inline static constexpr struct {
} ignore { };

}
