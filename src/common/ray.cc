#include "common/Ray.hpp"
#include <raymath.h>

namespace common::ray {
::Rectangle rec_centered_at(const ::Rectangle& rec, const ::Vector2& center)
{
    return ::Rectangle {
        .x = center.x - rec.width / 2.0f,
        .y = center.y - rec.height / 2.0f,
        .width = rec.width,
        .height = rec.height,
    };
}
::Vector2 rec_center(const ::Rectangle& rec)
{
    return ::Vector2Add(
        { rec.x, rec.y },
        ::Vector2Scale(
            { rec.width, rec.height },
            0.5f));
}
::Vector2 text_centered_at(const ::Vector2& center, const ::Vector2& text_dims)
{
    return ::Vector2 {
        .x = center.x - text_dims.x / 2.0f,
        .y = center.y - text_dims.y / 2.0f,
    };
}
}
