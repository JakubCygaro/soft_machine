#include "game/Scene.hpp"
#include "game/Globals.hpp"
#include <algorithm>
#include <iostream>
#include <memory>
#include <ostream>
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
    init(*this);
}

void GraphScene::init(GraphScene& self){
    for (auto c : self.m_mgraph->get_connections()) {
        if (auto o = dynamic_cast<Object*>(c.get()))
            self.m_graph_objs.push_back(o);
    }
    for (auto c : self.m_mgraph->get_components()) {
        if (auto o = dynamic_cast<Object*>(c.get()))
            self.m_graph_objs.push_back(o);
    }
    self.m_cam.target = { 0, 0 };
    self.m_cam.offset = { self.bounds.width / 2.0f, self.bounds.height / 2.0f };
    self.m_cam.zoom = 2.0;
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
    if (m_framec % 60 == 0) {
        m_mgraph->poll_all();
    }
    GraphScene::set_camera(&m_cam);
    for (auto o : m_graph_objs) {
        o->update();
    }
    m_framec++;
}
}
