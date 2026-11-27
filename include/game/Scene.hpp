#pragma once
#include "game/StylingGraph.hpp"
#include "machine/MachineGraph.hpp"
#include <chrono>
#include <cstdint>
#include <functional>
#include <raylib.h>

namespace game {
using namespace std::chrono_literals;
namespace chrono = std::chrono;
class GraphScene {
public:
    static constexpr chrono::milliseconds DEFAULT_POLL_RATE = 1000ms;
    using clock_t = chrono::steady_clock;
    using time_point_t = chrono::time_point<clock_t>;
    using graph_t = game::StylingGraph;
    GraphScene() = delete;
    GraphScene(const GraphScene&) = delete;
    GraphScene& operator=(const GraphScene&) = delete;
    explicit GraphScene(std::unique_ptr<graph_t>&&, ::Rectangle);
    GraphScene(GraphScene&&) = default;
    GraphScene& operator=(GraphScene&&) = default;
    ~GraphScene() = default;

private:
    static void init(GraphScene& self);
    static void calc_next_poll(GraphScene& self);
    std::unique_ptr<graph_t> m_mgraph;
    ::Camera2D m_cam;
    uint64_t m_framec { };
    time_point_t m_next_poll;
    bool m_paused { true };
    chrono::milliseconds m_poll_rate = DEFAULT_POLL_RATE;

public:
    ::Rectangle bounds { };
    bool m_has_focus { };
    bool m_input_to_elements { };

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
        // if (auto o = static_cast<Object*>(conn); o)
        //     this->m_graph_objs.push_back(o);
        return conn;
    }
    template <std::derived_from<machine::Component> T, typename... Args>
    inline T* create_component(Args&&... ctor_args)
    {
        auto conn = this
                        ->m_mgraph
                        ->create_component<T>(std::forward<Args>(ctor_args)...);
        // if (auto o = static_cast<Object*>(conn); o)
        //     this->m_graph_objs.push_back(o);
        return conn;
    }
#ifdef FACELIFT_EDITOR
    inline graph_t* get_graph()
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

    void set_focus(bool);
    void set_input_to_elements(bool);
    chrono::milliseconds get_poll_rate(void) const;
    void set_poll_rate(chrono::milliseconds);
    void set_pause(bool);
    bool is_paused(void) const;
    template <typename Ret>
    inline Ret modify_graph(std::function_ref<Ret(graph_t*)> func)
    {
        return func(this->m_mgraph.get());
    }
};
}
