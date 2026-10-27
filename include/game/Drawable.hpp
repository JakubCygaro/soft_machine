#pragma once

#include "game/Styling.hpp"
#include <memory>
#include <raylib.h>
#include <raymath.h>
namespace game {
struct Editable {
    inline virtual ~Editable() { }
    /// the return value indicates whether a change to the underlying object occured
    virtual bool draw_edit_window() = 0;
    /// handler for when the object is notified that something has changed
    inline virtual void on_notified() { };
};
class Object : public Editable, public Stylable {
private:
    std::shared_ptr<Style> m_styling;

public:
    inline virtual ~Object() = default;
    inline virtual void draw() { };
    inline virtual void update() { };
    inline virtual void on_input() { };
    // inline virtual void on_keyboard_event(){};
    inline virtual bool check_point_collision(const ::Vector2& point) const
    {
        (void)point;
        return false;
    };
    inline virtual void set_styling(std::shared_ptr<Style> s)
    {
        m_styling = s;
    }
    // styling
    inline virtual void set_pos(const ::Vector2&) override { };
    inline virtual void set_size(const ::Vector2&) override { };
    ::Vector2 get_pos() const override { return ::Vector2Zero(); }
    ::Vector2 get_size() const override { return ::Vector2Zero(); }
    ::Rectangle get_bounds() const override { return ::Rectangle { }; }

protected:
    inline virtual const std::shared_ptr<Style>& get_style()
    {
        return m_styling;
    };
    inline virtual const std::shared_ptr<Style>& get_style() const
    {
        return m_styling;
    };
};
}
