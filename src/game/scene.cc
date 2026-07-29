#include "game/Scene.hpp"
#include <algorithm>
#include <memory>
#include <raylib.h>
#include <raymath.h>
namespace game {
GraphScene::GraphScene(GraphScene&& o)
    : m_mgraph { std::move(o.m_mgraph) }
    , m_graph_objs { std::move(o.m_graph_objs) }
    , m_cam { o.m_cam }
{
}
GraphScene& GraphScene::operator=(GraphScene&& o)
{
    m_mgraph = std::move(o.m_mgraph);
    m_graph_objs = std::move(o.m_graph_objs);
    m_cam = o.m_cam;
    bounds = o.bounds;
    return *this;
}
GraphScene::~GraphScene()
{
}
GraphScene::GraphScene(machine::MachineGraph&& g, ::Rectangle bounds)
    : m_mgraph { std::move(g) }
    , m_cam { }
    , bounds { bounds }
{
    for (auto c : m_mgraph.get_connections()) {
        if (auto o = std::dynamic_pointer_cast<Object>(c))
            this->m_graph_objs.push_back(o);
    }
    for (auto c : m_mgraph.get_components()) {
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
    ::ClearBackground(::ColorFromHSV(60, 19, 200));
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
    for (auto o : m_graph_objs) {
        o->update();
    }
}
}
