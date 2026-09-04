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
    static void init(GraphScene& self);
    std::unique_ptr<machine::MachineGraph> m_mgraph;
    std::vector<Object*> m_graph_objs { };
    ::Camera2D m_cam;
    uint64_t m_framec { };

public:
    ::Rectangle bounds { };

public:
    void draw();
    void update();
    template <std::derived_from<machine::Connection> T, typename... Args>
    inline T* create_connection(
        std::string name,
        std::string from,
        std::string to,
        Args&&... ctor_args)
    {
        auto conn = this
                        ->m_mgraph
                        ->create_connection<T>(
                            name, from, to, std::forward<Args>(ctor_args)...);
        if (auto o = static_cast<Object*>(conn); o)
            this->m_graph_objs.push_back(o);
        return conn;
    }
    template <std::derived_from<machine::Component> T, typename... Args>
    inline T* create_component(Args&&... ctor_args)
    {
        auto conn = this
                        ->m_mgraph
                        ->create_component<T>(std::forward<Args>(ctor_args)...);
        if (auto o = static_cast<Object*>(conn); o)
            this->m_graph_objs.push_back(o);
        return conn;
    }
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
    inline const ::Camera2D* get_instance_camera()
    {
        return &m_cam;
    }
};
}
