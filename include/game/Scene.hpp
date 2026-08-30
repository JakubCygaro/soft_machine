#pragma once
#include "game/Drawable.hpp"
#include "machine/MachineGraph.hpp"
#include <cstdint>
#include <memory>
#include <raylib.h>
#include <vector>

namespace game {
class GraphScene {
public:
    GraphScene() = delete;
    GraphScene(const GraphScene&) = delete;
    GraphScene& operator=(const GraphScene&) = delete;
    explicit GraphScene(std::unique_ptr<machine::MachineGraph>&&, ::Rectangle);
    GraphScene(GraphScene&&);
    GraphScene& operator=(GraphScene&&);
    ~GraphScene();

private:
    std::unique_ptr<machine::MachineGraph> m_mgraph;
    std::vector<std::shared_ptr<Object>> m_graph_objs { };
    ::Camera2D m_cam;
    uint64_t m_framec { };

public:
    ::Rectangle bounds { };

public:
    void draw();
    void update();
#ifdef FACELIFT_EDITOR
    inline machine::MachineGraph* get_graph()
    {
        return m_mgraph.get();
    }
#endif

private:
    inline static ::Camera2D* camera2D = nullptr;
    inline static void set_camera(::Camera2D* c)
    {
        camera2D = c;
    }

public:
    inline static const ::Camera2D* get_camera()
    {
        return camera2D;
    }
};
}
