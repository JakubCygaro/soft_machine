#ifndef MACHINE_GRAPH_HPP
#define MACHINE_GRAPH_HPP
#include "machine/Actor.hpp"
#include "machine/Component.hpp"
#include "machine/Connection.hpp"
#include "machine/MachineContext.hpp"
#include "machine/Message.hpp"
#include "machine/Scheduler.hpp"
#include <concepts>
#include <deque>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace machine {
class MachineContext;

class MachineGraph : public shed::Scheduler {
private:
    std::list<std::shared_ptr<Component>> m_comps { };
    std::unordered_map<std::string, Component*> m_named_comps { };
    std::list<std::shared_ptr<Connection>> m_conns { };
    std::unordered_map<std::string, Connection*> m_named_conns { };

    struct MessageSent {
        std::string sender;
        std::string recipent;
        message_t payload;
        shed::send_callback_t sender_callback;
    };
    std::deque<MessageSent> m_msgq { };

    std::unordered_map<std::string, shed::recv_callback_t> m_waiting { };

    struct Process {
        std::string name;
        actor::Actor actor;
        Pollable* pollable;
    };

    std::deque<Process> m_procs { };
    std::deque<actor::Actor::handle_t> m_scheduled { };

public:
    MachineGraph();
    MachineGraph(const MachineGraph&) = delete;
    MachineGraph& operator=(const MachineGraph&) = delete;
    MachineGraph(MachineGraph&&);
    MachineGraph& operator=(MachineGraph&&);
    ~MachineGraph();

private:
    template <std::derived_from<Pollable> T>
    inline void register_actor(std::string with_name, T* pollable)
    {
        using namespace std::placeholders;
        MachineContext mctx = MachineContext(with_name, this);
        auto act = pollable->poll(mctx);
        this->m_procs.push_back(
            Process {
                .name = with_name,
                .actor = std::move(act),
                .pollable = pollable });
    }

public:
    template <std::derived_from<Connection> T, typename... Args>
    inline T* create_connection(
        std::string name,
        std::string from,
        std::string to,
        Args&&... ctor_args)
    {
        if (m_named_conns.contains(name)) {
            throw std::runtime_error("connection already exists");
        }
        if (!m_named_comps.contains(from))
            throw std::runtime_error("from component does not exist");
        if (!m_named_comps.contains(to))
            throw std::runtime_error("to component does not exist");

        auto* from_ptr = m_named_comps[from];
        auto* to_ptr = m_named_comps[to];
        std::shared_ptr<T> conn = nullptr;
        if constexpr (sizeof...(ctor_args) > 0) {
            conn = std::make_shared<T>(from_ptr,
                to_ptr,
                from,
                to,
                std::forward<Args>(ctor_args)...);
        } else {
            conn = std::make_shared<T>(from_ptr, to_ptr, from, to);
        }
        auto [d1, c1] = conn->on_connecting_to_start();
        c1(
            from_ptr->on_incoming_connection(name, conn.get(), d1));
        auto [d2, c2] = conn->on_connecting_to_end();
        c2(
            to_ptr->on_outcoming_connection(name, conn.get(), d2));
        m_named_conns[name] = conn.get();
        m_conns.push_back(conn);
        register_actor(name, conn.get());
        return conn.get();
    }
    template <std::derived_from<Component> T, typename... Args>
    inline T* create_component(Args&&... ctor_args)
    {
        std::shared_ptr<T> comp = nullptr;
        if constexpr (sizeof...(ctor_args) > 0) {
            comp = std::make_shared<T>(std::forward<Args>(ctor_args)...);
        } else {
            comp = std::make_shared<T>();
        }
        const auto name = comp->get_name();
        if (m_named_comps.contains(name)) {
            throw std::runtime_error("component already exists");
        }
        m_named_comps[name] = comp.get();
        m_comps.push_back(comp);
        register_actor(name, comp.get());
        return comp.get();
    }

private:
    void deliver_messages();
    bool is_connector(const std::string&) const;
    bool is_component(const std::string&) const;

public:
    void poll_all();

    using comp_ref = const std::list<std::shared_ptr<Component>>&;
    using conn_ref = const std::list<std::shared_ptr<Connection>>&;

    comp_ref get_components() const;
    conn_ref get_connections() const;
    inline auto get_components_begin() -> auto
    {
        return this->m_comps.begin();
    }
    inline auto get_components_end() -> auto
    {
        return this->m_comps.end();
    }
    inline auto get_connections_begin() -> auto
    {
        return this->m_conns.begin();
    }
    inline auto get_connections_end() -> auto
    {
        return this->m_conns.end();
    }

    // As Scheduler
public:
    virtual void pause(machine::actor::Actor::handle_t) override;
    virtual void send(
        std::string sender,
        std::string recipent,
        message_t,
        shed::send_callback_t) override;
    virtual void recv(
        std::string who,
        shed::recv_callback_t) override;
};
}

#endif
