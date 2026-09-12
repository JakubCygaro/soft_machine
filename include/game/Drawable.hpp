#pragma once

#include <raylib.h>
namespace game {
class Object {
public:
    inline virtual void draw() { };
    inline virtual void update() { };
    inline virtual void on_input() { };
    // inline virtual void on_keyboard_event(){};
    inline virtual bool check_point_collision(const ::Vector2& point) const
    {
        (void)point;
        return false;
    };
};
}
