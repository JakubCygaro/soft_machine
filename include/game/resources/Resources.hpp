#pragma once

#include <raylib.h>
namespace game::resources {

inline constexpr ::Vector2 get_default_node_size()
{
    return ::Vector2 {
        .x = 50,
        .y = 50,
    };
}
inline constexpr float default_node_font_size()
{
    return 24;
}
inline constexpr float default_font_spacing()
{
    return 10;
}

::Font get_node_font();
void init_resources();
void deinit_resources();

}
