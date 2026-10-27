#pragma once

#include <raylib.h>
namespace common::ray {
::Rectangle rec_centered_at(const ::Rectangle& rec, const ::Vector2& center);
::Vector2 rec_center(const ::Rectangle&);
::Vector2 text_centered_at(const ::Vector2& center, const ::Vector2& text_dims);
}
