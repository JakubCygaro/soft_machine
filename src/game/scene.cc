#include "game/Scene.hpp"
#include "game/Globals.hpp"
#include <algorithm>
#include <cmath>
#include <format>
#include <memory>
#include <raylib.h>
#include <raymath.h>
namespace game {
// GraphScene::GraphScene(GraphScene&& o)
//     : m_mgraph { std::move(o.m_mgraph) }
//     // , m_graph_objs { std::move(o.m_graph_objs) }
//     , m_cam { o.m_cam }
//     , m_framec { o.m_framec }
//     , bounds { o.bounds }
//     , m_has_focus { o.m_has_focus }
//     , m_input_to_elements { o.m_input_to_elements }
// {
// }
// GraphScene& GraphScene::operator=(GraphScene&& o)
// {
//     if (&o == this)
//         return *this;
//     m_mgraph = std::move(o.m_mgraph);
//     m_cam = o.m_cam;
//     bounds = o.bounds;
//     m_framec = o.m_framec;
//     m_has_focus = o.m_has_focus;
//     m_input_to_elements = o.m_input_to_elements;
//     return *this;
// }
// GraphScene::~GraphScene()
// {
// }
GraphScene::GraphScene(std::unique_ptr<graph_t>&& g, ::Rectangle bounds)
    : m_mgraph { std::move(g) }
    , m_cam { }
    , m_next_poll { chrono::steady_clock::now() }
    , bounds { bounds }
    , m_has_focus { false }
    , m_input_to_elements { false }
{
    init(*this);
}

void GraphScene::init(GraphScene& self)
{
    self.m_cam.target = { 0, 0 };
    self.m_cam.offset = { self.bounds.width / 2.0f, self.bounds.height / 2.0f };
    self.m_cam.zoom = 2.0;
    calc_next_poll(self);
}
void GraphScene::calc_next_poll(GraphScene& self)
{
    self.m_next_poll = clock_t::now() + self.m_poll_rate;
}

void GraphScene::draw()
{
    ::BeginScissorMode(bounds.x, bounds.y, bounds.width, bounds.height);
    ::ClearBackground(::GetColor(0xffffc1ff));
    ::BeginMode2D(m_cam);
    for (auto o : this->m_mgraph->get_connections()) {
        o->draw();
    }
    for (auto o : this->m_mgraph->get_components()) {
        o->draw();
    }
    ::EndMode2D();
    ::EndScissorMode();
}
void GraphScene::update()
{
    if (m_has_focus && ::CheckCollisionPointRec(::GetMousePosition(), bounds)) {
        m_cam.zoom = std::exp(std::log(m_cam.zoom) + (::GetMouseWheelMove() * 0.1f));
        m_cam.zoom = std::clamp<float>(m_cam.zoom, 0.0, 75.0);
        if (::IsMouseButtonDown(::MOUSE_BUTTON_RIGHT)) {
            m_cam.target = ::Vector2Subtract(m_cam.target,
                ::Vector2Scale(::GetMouseDelta(), 1 / m_cam.zoom));
        }
    }
    if (!m_paused && clock_t::now() >= m_next_poll) {
        m_mgraph->poll_all();
        calc_next_poll(*this);
    }
    GraphScene::set_camera(&m_cam);
    const auto m = ::GetScreenToWorld2D(::GetMousePosition(), m_cam);
    for (auto o : m_mgraph->get_elements_as<Object>()) {
        if (m_has_focus
            && m_input_to_elements
            && o->check_point_collision(m)) {
            o->on_input();
        }
        o->update();
    }
    m_framec++;
}
void GraphScene::set_focus(bool f)
{
    m_has_focus = f;
}
void GraphScene::set_input_to_elements(bool in)
{
    m_input_to_elements = in;
}
chrono::milliseconds GraphScene::get_poll_rate(void) const
{
    return m_poll_rate;
}
void GraphScene::set_poll_rate(chrono::milliseconds poll_rate)
{
    if (m_poll_rate < 0ms) {
        throw std::runtime_error(
            std::format("GraphScene::set_poll_rate: negative value of '{}' passed",
                poll_rate));
    }
    // correct next poll, if new poll rate is further in time then the difference
    // will be poisitive and the next poll will get postponed
    const auto diff = poll_rate - m_poll_rate;
    m_next_poll += diff;
    m_poll_rate = poll_rate;
}
void GraphScene::set_pause(bool p)
{
    m_paused = p;
    if (!p)
        m_next_poll = clock_t::now();
}
bool GraphScene::is_paused(void) const
{
    return m_paused;
}
void GraphScene::step_poll(void)
{
    if (!m_paused)
        return;
    m_mgraph->poll_all();
}
}
