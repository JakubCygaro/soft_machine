#pragma once
#include "Pollable.hpp"
#include "machine/Actor.hpp"
#include "machine/Connection.hpp"
#include "machine/Message.hpp"
#include "machine/Preamble.hpp"
#include "machine/Scheduler.hpp"
#include <algorithm>
#include <concepts>
#include <deque>
#include <format>
#include <list>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace machine {
template <
    std::derived_from<Component> Comp,
    std::derived_from<Connection> Conn>
class MachineGraph : public shed::Scheduler {
private:
    std::list<std::shared_ptr<Comp>> m_comps { };
    std::unordered_map<std::string, Comp*>
        m_named_comps { };
    std::list<std::shared_ptr<Conn>> m_conns { };
    std::unordered_map<std::string, Conn*>
        m_named_conns { };
    std::unordered_map<Comp*, std::vector<Conn*>> m_incidents { };

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
        actor::Actor::handle_t handle;
        Pollable* pollable;
        inline Process(
            std::string name,
            actor::Actor&& actor,
            Pollable* pollable)
            : name { name }
            , actor { std::move(actor) }
            , handle { std::move(handle) }
            , pollable { pollable }
        {
        }
        inline Process(const Process&) = delete;
        inline Process& operator=(const Process&) = delete;
        inline Process(Process&& o)
            : name { o.name }
            , actor { std::move(o.actor) }
            , handle { std::move(handle) }
            , pollable { o.pollable }
        {
            o.pollable = nullptr;
            o.handle = nullptr;
        }
        inline Process& operator=(Process&& o)
        {
            name = o.name;
            actor = std::move(o.actor);
            handle = std::move(o.handle);
            pollable = o.pollable;
            o.pollable = nullptr;
            o.handle = nullptr;
            return *this;
        }
    };

    std::deque<Process> m_procs { };
    std::deque<std::pair<const std::string, shed::pause_callback_t>> m_paused { };

private:
    inline MachineGraph() { }

public:
    inline static MachineGraph* create()
    {
        return new MachineGraph();
    }
    MachineGraph(const MachineGraph&) = delete;
    MachineGraph& operator=(const MachineGraph&) = delete;
    inline MachineGraph(MachineGraph&& o)
        : m_comps { std::move(o.m_comps) }
        , m_named_comps { std::move(o.m_named_comps) }
        , m_conns { std::move(o.m_conns) }
        , m_named_conns { std::move(o.m_named_conns) }
        , m_incidents { std::move(o.m_incidents) }
        , m_msgq { std::move(o.m_msgq) }
        , m_waiting { std::move(o.m_waiting) }
        , m_procs { std::move(o.m_procs) }
        , m_paused { std::move(o.m_paused) }
    {
    }
    inline MachineGraph& operator=(MachineGraph&& o)
    {
        m_comps = std::move(o.m_comps);
        m_named_comps = std::move(o.m_named_comps);
        m_conns = std::move(o.m_conns);
        m_named_conns = std::move(o.m_named_conns);
        m_msgq = std::move(o.m_msgq);
        m_waiting = std::move(o.m_waiting);
        m_procs = std::move(o.m_procs);
        m_paused = std::move(o.m_paused);
        m_incidents = std::move(o.m_incidents);
        return *this;
    }
    inline virtual ~MachineGraph()
    {
    }

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
    template <std::derived_from<Conn> T, typename... Args>
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
    template <std::derived_from<Comp> T, typename... Args>
    inline T* create_component(Args&&... ctor_args)
    {
        std::shared_ptr<T> comp = nullptr;
        if constexpr (sizeof...(ctor_args) > 0) {
            comp = std::make_shared<T>(std::forward<Args>(ctor_args)...);
        } else {
            comp = std::make_shared<T>();
        }
        const auto name = comp->get_name();
        if (!m_named_comps.empty() && m_named_comps.contains(name)) {
            throw std::runtime_error("component already exists");
        }
        m_named_comps[name] = comp.get();
        m_comps.push_back(comp);
        register_actor(name, comp.get());
        return comp.get();
    }
    inline void remove_element(const std::string& name)
    {
        auto found = std::find_if(
            m_procs.begin(),
            m_procs.end(),
            [&](auto& proc) {
                return proc.name == name;
            });
        if (found != m_procs.end())
            m_procs.erase(found);

        if (is_component(name)) {
            m_named_comps.erase(name);
            m_comps.erase(
                std::find_if(
                    m_comps.begin(),
                    m_comps.end(),
                    [&](auto& comp) {
                        return comp->get_name() == name;
                    }));
        }
        if (is_connector(name)) {
            m_named_conns.erase(name);
            m_conns.erase(
                std::find_if(
                    m_conns.begin(),
                    m_conns.end(),
                    [&](auto& conn) {
                        return conn->get_name() == name;
                    }));
        }
    }

private:
    inline void deliver_messages()
    {
        for (auto i = m_msgq.size(); i > 0; i--) {
            auto ms = std::move(m_msgq.front());
            m_msgq.pop_front();
            if (!m_waiting.contains(ms.recipent)) {
                m_msgq.push_back(std::move(ms));
                continue;
            }
            if (!exists(ms.sender))
                continue;
            // cannot send from comp to comp
            if (is_component(ms.recipent) && is_component(ms.sender)) {
                ms.sender_callback(
                    std::runtime_error("attempted to message another component directly"));
                continue;
            }
            if (is_connector(ms.recipent) && is_connector(ms.sender)) {
                ms.sender_callback(
                    std::runtime_error("attempted to message another connector directly"));
                continue;
            }
            Connection* conn { };
            Component* comp { };
            if (is_connector(ms.recipent)) {
                conn = m_named_conns[ms.recipent];
                comp = m_named_comps[ms.sender];
            } else {
                conn = m_named_conns[ms.sender];
                comp = m_named_comps[ms.recipent];
            }
            if (conn->get_end() != comp && conn->get_start() != comp) {
                ms.sender_callback(
                    std::runtime_error("reciever is not connected to this element"));
                continue;
            }
            m_waiting[ms.recipent](ms.sender, std::move(ms.payload));
            m_waiting.erase(ms.recipent);
            ms.sender_callback(std::nullopt);
        }
    }
    inline bool exists(const std::string& n) const
    {
        return is_component(n) || is_connector(n);
    }
    inline bool is_connector(const std::string& n) const
    {
        return m_named_conns.contains(n);
    }
    inline bool is_component(const std::string& n) const
    {
        return m_named_comps.contains(n);
    }

public:
    using ahandle_t = machine::actor::Actor::handle_t;

    inline void poll_all()
    {
        static bool once;
        if (!once) {
            for (auto& proc : m_procs) {
                proc.actor.resume();
            }
            once = true;
            return;
        }
        deliver_messages();
        decltype(m_paused) scheduled = decltype(m_paused)(m_paused);
        // decltype(m_paused) scheduled;
        // decltype(m_paused)::swap(m_paused, scheduled);
        m_paused.clear();
        while (!scheduled.empty()) {
            auto [n, wake] = scheduled.front();
            scheduled.pop_front();
            if (exists(n)) {
                wake();
            }
            // h.resume();
        }
    }
    using incident_t = std::vector<Conn*>;
    inline std::optional<incident_t*>
    get_incident_to(const std::string& name)
    {
        if (!this->m_named_comps.contains(name)) {
            return std::nullopt;
        }
        const auto ptr = this->m_named_comps.at(name);
        if (!this->m_incidents.contains(ptr)) {
            return std::nullopt;
        }
        const auto in = &this->m_incidents.at(ptr);
        return std::make_optional(in);
    }
    using adjecent_t = std::vector<Comp*>;
    inline std::optional<adjecent_t>
    get_adjecent_to(const std::string& name)
    {
        const auto incident = get_incident_to(name);
        if (!incident.has_value())
            return std::nullopt;
        const auto n = m_named_comps.at(name);
        std::vector<const Component*> ret { };
        for (const auto i : **incident) {
            if (i->get_end() != n) {
                ret.push_back(i->get_end());
            } else {
                ret.push_back(i->get_start());
            }
        }
        return std::make_optional(ret);
    }

    using comp_ref = const std::list<std::shared_ptr<Comp>>&;
    using conn_ref = const std::list<std::shared_ptr<Conn>>&;

    inline comp_ref get_components() const
    {
        return this->m_comps;
    }
    inline conn_ref get_connections() const
    {
        return this->m_conns;
    }
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
    using comp_or_conn_ptr_t = std::variant<Comp*, Conn*>;
    inline std::optional<Component*> query_component(const std::string& sv)
    {
        if (this->m_named_comps.contains(sv)) {
            return m_named_comps[sv];
        }
        return std::nullopt;
    }
    inline std::optional<Connection*> query_connection(const std::string& sv)
    {
        if (this->m_named_conns.contains(sv)) {
            return m_named_conns[sv];
        }
        return std::nullopt;
    }
    inline std::optional<MachineGraph::comp_or_conn_ptr_t>
    query_element(const std::string& sv)
    {
        if (auto comp = query_component(sv); comp) {
            return MachineGraph::comp_or_conn_ptr_t { *comp };
        } else if (auto conn = query_connection(sv); conn) {
            return MachineGraph::comp_or_conn_ptr_t { *conn };
        }
        return std::nullopt;
    }

    inline auto get_elements() -> auto
    {
        using namespace std::views;
        const auto t = [](auto& c) {
            return comp_or_conn_ptr_t { c.get() };
        };
        return concat(transform(m_comps, t), transform(m_conns, t));
    }
    template <typename T>
    inline auto get_elements_as() -> auto
    {
        static_assert(std::derived_from<Comp, T> && std::derived_from<Conn, T>);
        using namespace std::views;
        const auto t = [](auto& c) {
            return static_cast<T*>(c.get());
        };
        return concat(transform(m_comps, t), transform(m_conns, t));
    }

    // As Scheduler
public:
    inline virtual void pause(const std::string& name, shed::pause_callback_t clb)
    {
        m_paused.push_back(std::make_pair(name, clb));
    }
    inline virtual void send(
        std::string sender,
        std::string recipent,
        message_t msg,
        shed::send_callback_t c)
    {
        m_msgq.push_back(
            MessageSent {
                .sender = sender,
                .recipent = recipent,
                .payload = std::move(msg),
                .sender_callback = c });
    }
    inline virtual void recv(
        std::string who,
        shed::recv_callback_t c)
    {
        m_waiting[who] = c;
    }
};
}
