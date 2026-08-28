#include "components/Display.hpp"
#include <string>

namespace components {

void Display::draw()
{
    const auto center = get_center();
    ::DrawRectangleRounded(
        this->m_bounds,
        0.2,
        10,
        BODY_COLOR);
    const ::Vector2 screen_d = {
        .x = m_bounds.width * .8f,
        .y = m_bounds.height * .8f,
    };
    const ::Vector2 screen_p = {
        .x = center.x - screen_d.x / 2.0f,
        .y = center.y - screen_d.y / 2.0f,
    };
    ::DrawRectangleRec(
        {
            .x = screen_p.x,
            .y = screen_p.y,
            .width = screen_d.x,
            .height = screen_d.y,
        },
        SCREEN_COLOR);
    const ::Vector2 val_p = {
        .x = center.x - m_d->val_dims.x / 2.0f,
        .y = center.y - m_d->val_dims.y / 2.0f,
    };
    using namespace game::resources;
    if(m_d->val){
        // ::BeginScissorMode(
        //     screen_p.x,
        //     screen_p.y,
        //     screen_d.x,
        //     screen_d.y);
        ::DrawTextEx(
            get_node_font(),
            m_d->val_rep.c_str(),
            val_p,
            m_d->font_size,
            default_font_spacing(),
            ::WHITE);
        // ::EndScissorMode();
    }
}
void Display::update()
{
}
machine::actor::Actor Display::poll(machine::Mctx ctx)
{
    using namespace game::resources;
    while (1) {
        auto [s, m] = co_await ctx.recv();
        if (auto w = std::any_cast<Display::display_msg_t>(&m); w) {
            m_d->val = w->val;
        }
        if (auto w = std::any_cast<int>(&m); w) {
            m_d->val = *w;
        }
        if (m_d->val) {
            m_d->val_rep = std::to_string(*m_d->val);
            m_d->val_dims = ::MeasureTextEx(
                get_node_font(),
                m_d->val_rep.c_str(),
                m_d->font_size,
                default_font_spacing());
        }
    }
}

void Display::set_size(const ::Vector2& sz)
{
    m_bounds.width = sz.x;
    m_bounds.height = sz.y;
}
}
