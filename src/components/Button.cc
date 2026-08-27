#include "components/Button.hpp"
#include "game/Scene.hpp"
#include <raylib.h>
namespace components {

void Button::draw()
{
    const auto center = get_center();
    ::DrawRectangleRounded(
        this->m_bounds,
        0.2,
        10,
        BODY_COLOR);
    const ::Vector2 button_d = {
        .x = m_bounds.width * .8f,
        .y = m_bounds.height * .8f,
    };
    const ::Vector2 button_p = {
        .x = center.x - button_d.x / 2.0f,
        .y = center.y - button_d.y / 2.0f,
    };
    const auto c = (d->set) ? BUTTON_ACTIVE_COLOR : BUTTON_INACTIVE_COLOR;
    ::DrawRectangleRounded(
        {
            .x = button_p.x,
            .y = button_p.y,
            .width = button_d.x,
            .height = button_d.y,
        },
        0.2,
        10,
        c);
}
void Button::update()
{
    const auto m = ::GetScreenToWorld2D(::GetMousePosition(),
        *game::GraphScene::get_camera());
    if (::IsMouseButtonReleased(::MOUSE_BUTTON_LEFT)
        && ::CheckCollisionPointRec(m, m_bounds)
        && !d->set) {
        d->set = true;
    }
}
std::any Button::on_outcoming_connection(
    std::string_view conn_name,
    const machine::Connection*,
    std::any)
{
    this->d->recipents.push_back(std::string(conn_name));
    return nullptr;
}
machine::actor::Actor Button::poll(machine::Mctx ctx)
{
    while (1) {
        co_await ctx.pause();
        if (this->d->set) {
            for (const auto& rc : this->d->recipents) {
                auto msg = std::any(d->m_msg_value);
                co_await ctx.send(rc, std::move(msg));
            }
            this->d->set = false;
        }
    }
}
void Button::set_size(const ::Vector2& sz)
{
    m_bounds.width = sz.x;
    m_bounds.height = sz.y;
}
}
