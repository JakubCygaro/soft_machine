#pragma once

#include <raylib.h>
namespace game {
struct Editable {
    inline virtual ~Editable() { }
    /// the return value indicates whether a change to the underlying object occured
    virtual bool draw_edit_window() = 0;
    /// handler for when the object is notified that something has changed
    inline virtual void on_notified() { };
};
class Object : public Editable {
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
