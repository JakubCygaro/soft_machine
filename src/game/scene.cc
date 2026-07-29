#include "game/Scene.hpp"
#include <algorithm>
#include <memory>
#include <print>
#include <raylib.h>
#include <raymath.h>
namespace game {
GraphScene::GraphScene(GraphScene&& o)
    : m_mgraph { std::move(o.m_mgraph) }
    , m_graph_objs { std::move(o.m_graph_objs) }
    , m_cam { o.m_cam }
    , m_framec { o.m_framec }
{
}
GraphScene& GraphScene::operator=(GraphScene&& o)
{
    m_mgraph = std::move(o.m_mgraph);
    m_graph_objs = std::move(o.m_graph_objs);
    m_cam = o.m_cam;
    bounds = o.bounds;
    m_framec = o.m_framec;
    return *this;
}
GraphScene::~GraphScene()
{
}
GraphScene::GraphScene(std::unique_ptr<machine::MachineGraph>&& g, ::Rectangle bounds)
    : m_mgraph { std::move(g) }
    , m_cam { }
    , bounds { bounds }
{
    for (auto c : m_mgraph->get_connections()) {
        if (auto o = std::dynamic_pointer_cast<Object>(c))
            this->m_graph_objs.push_back(o);
    }
    for (auto c : m_mgraph->get_components()) {
        if (auto o = std::dynamic_pointer_cast<Object>(c))
            this->m_graph_objs.push_back(o);
    }
    m_cam.target = { 0, 0 };
    m_cam.offset = { bounds.width / 2.0f, bounds.height / 2.0f };
    m_cam.zoom = 2.0;
}

void GraphScene::draw()
{
    ::BeginScissorMode(bounds.x, bounds.y, bounds.width, bounds.height);
    ::ClearBackground(::GetColor(0xffffc1ff));
    ::BeginMode2D(m_cam);
    for (auto o : m_graph_objs) {
        o->draw();
    }
    ::EndMode2D();
    ::EndScissorMode();
}
void GraphScene::update()
{
    if (::CheckCollisionPointRec(::GetMousePosition(), bounds)) {
        m_cam.zoom += ::GetMouseWheelMove() * .3;
        m_cam.zoom = std::clamp<float>(m_cam.zoom, 0.0, 100.0);
        if (::IsMouseButtonDown(::MOUSE_BUTTON_RIGHT)) {
            m_cam.target = ::Vector2Subtract(m_cam.target,
                ::Vector2Scale(::GetMouseDelta(), 1 / m_cam.zoom));
        }
    }
    if(m_framec % 60 == 0){
        m_mgraph->poll_all();
    }
    for (auto o : m_graph_objs) {
        o->update();
    }
    m_framec++;
}
}
