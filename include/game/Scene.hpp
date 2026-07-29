#pragma once
#include "game/Drawable.hpp"
#include "machine/MachineGraph.hpp"
#include <memory>
#include <raylib.h>
#include <vector>

namespace game {
class GraphScene {
public:
    GraphScene() = delete;
    GraphScene(const GraphScene&) = delete;
    GraphScene& operator=(const GraphScene&) = delete;
    explicit GraphScene(machine::MachineGraph&&, ::Rectangle);
    GraphScene(GraphScene&&);
    GraphScene& operator=(GraphScene&&);
    ~GraphScene();

private:
    machine::MachineGraph m_mgraph;
    std::vector<std::shared_ptr<Object>> m_graph_objs { };
    ::Camera2D m_cam;

public:
    ::Rectangle bounds { };

public:
    void draw();
    void update();
};
}
