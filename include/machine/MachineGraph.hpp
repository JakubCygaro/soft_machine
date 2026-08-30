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
#include <format>
#include <functional>
#include <list>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace machine {
class MachineGraph : public shed::Scheduler {
private:
    std::list<std::shared_ptr<Component>> m_comps { };
    std::unordered_map<std::string, Component*>
        m_named_comps { };
    std::list<std::shared_ptr<Connection>> m_conns { };
    std::unordered_map<std::string, Connection*>
        m_named_conns { };
    std::unordered_map<Component*, std::vector<Connection*>> m_incidents { };

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
        inline Process(
            std::string name,
            actor::Actor&& actor,
            Pollable* pollable)
            : name { name }
            , actor { std::move(actor) }
            , pollable { pollable }
        {
        }
        inline Process(const Process&) = delete;
        inline Process& operator=(const Process&) = delete;
        inline Process(Process&& o)
            : name { o.name }
            , actor { std::move(o.actor) }
            , pollable { o.pollable }
        {
            o.pollable = nullptr;
        }
        inline Process& operator=(Process&& o)
        {
            name = o.name;
            actor = std::move(o.actor);
            pollable = o.pollable;
            o.pollable = nullptr;
            return *this;
        }
    };

    std::deque<Process> m_procs { };
    std::deque<actor::Actor::handle_t> m_scheduled { };

private:
    MachineGraph();

public:
    inline static MachineGraph* create()
    {
        return new MachineGraph();
    }
    MachineGraph(const MachineGraph&) = delete;
    MachineGraph& operator=(const MachineGraph&) = delete;
    MachineGraph(MachineGraph&&);
    MachineGraph& operator=(MachineGraph&&);
    virtual ~MachineGraph();

private:
    template <std::derived_from<Pollable> T>
    inline void register_actor(std::string with_name, T* pollable)
    {
        using namespace std::placeholders;
        MachineContext mctx = MachineContext(with_name, this);
        auto act = pollable->poll(mctx);
        this->m_procs.push_back(
            Process(
                with_name,
                std::move(act),
                pollable));
    }

public:
    template <std::derived_from<Connection> T, typename... Args>
    inline T* create_connection(
        std::string name,
        std::string from,
        std::string to,
        Args&&... ctor_args)
    {
        if (name.empty())
            throw std::runtime_error(
                std::format("attempted to create connection with empty name"));
        if (m_named_conns.contains(name)) {
            throw std::runtime_error(
                std::format("'{}' connection already exists",
                    name));
        }
        if (!m_named_comps.contains(from))
            throw std::runtime_error(
                std::format("{} from component '{}' does not exist",
                    name, from));
        if (!m_named_comps.contains(to))
            throw std::runtime_error(
                std::format("{} from component '{}' does not exist",
                    name, to));

        auto* from_ptr = m_named_comps[from];
        auto* to_ptr = m_named_comps[to];
        std::shared_ptr<T> conn = nullptr;
        if constexpr (sizeof...(ctor_args) > 0) {
            conn = std::make_shared<T>(
                name,
                from_ptr,
                to_ptr,
                from,
                to,
                std::forward<Args>(ctor_args)...);
        } else {
            conn = std::make_shared<T>(name, from_ptr, to_ptr, from, to);
        }
        auto [d1, c1] = conn->on_connecting_to_start();
        c1(
            from_ptr->on_outcoming_connection(name, conn.get(), d1));
        auto [d2, c2] = conn->on_connecting_to_end();
        c2(
            to_ptr->on_incoming_connection(name, conn.get(), d2));
        m_named_conns[name] = conn.get();
        if (!m_incidents.contains(from_ptr)) {
            m_incidents[from_ptr] = { conn.get() };
        } else {
            m_incidents[from_ptr].push_back(conn.get());
        }
        if (!m_incidents.contains(to_ptr)) {
            m_incidents[to_ptr] = { conn.get() };
        } else {
            m_incidents[to_ptr].push_back(conn.get());
        }
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
    std::optional<const std::vector<Connection*>*>
    get_incident_to(const std::string&) const;
    std::optional<std::vector<const Component*>>
    get_adjecent_to(const std::string&) const;

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
    std::optional<Component*> query_component(const std::string&);
    std::optional<Connection*> query_connection(const std::string&);
    using comp_or_conn_ptr_t = std::variant<Component*, Connection*>;
    std::optional<comp_or_conn_ptr_t> query_element(const std::string&);

    inline auto get_elements() -> auto
    {
        using namespace std::views;
        const auto t = [](auto& c) {
            return comp_or_conn_ptr_t { c.get() };
        };
        return concat(transform(m_comps, t), transform(m_conns, t));
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
