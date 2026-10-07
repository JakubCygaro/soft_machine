#pragma once
#include "Pollable.hpp"
#include "machine/Actor.hpp"
#include "machine/Connection.hpp"
#include "machine/Message.hpp"
#include "machine/Preamble.hpp"
#include "machine/Scheduler.hpp"
#include <algorithm>
#include <concepts>
#include <cstdint>
#include <deque>
#include <format>
#include <list>
#include <memory>
#include <optional>
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
    using proc_id_t = std::uint32_t;
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
        message_t payload;
        shed::send_callback_t sender_callback;
    };

    std::unordered_map<std::string, shed::recv_callback_t> m_waiting { };

    struct Process {
        // std::string name;
        actor::Actor actor;
        Pollable* pollable;
        std::deque<MessageSent> msgq { };
        std::optional<shed::recv_callback_t> awaiting_msg { };
        std::optional<shed::pause_callback_t> paused { };
        inline Process(
            // std::string name,
            actor::Actor&& actor,
            Pollable* pollable)
            : // name { name }
            actor { std::move(actor) }
            // , handle { std::move(handle) }
            , pollable { pollable }
        {
        }
        inline Process(const Process&) = delete;
        inline Process& operator=(const Process&) = delete;
        inline Process(Process&& o)
            // : name { o.name }
            : actor { std::move(o.actor) }
            // , handle { std::move(handle) }
            , pollable { o.pollable }
            , msgq { std::move(msgq) }
            , awaiting_msg { std::move(awaiting_msg) }
            , paused { std::move(paused) }
        {
            o.pollable = nullptr;
            // o.handle = nullptr;
        }
        inline Process& operator=(Process&& o)
        {
            actor = std::move(o.actor);
            msgq = std::move(o.msgq);
            awaiting_msg = std::move(o.awaiting_msg);
            paused = std::move(o.paused);
            pollable = o.pollable;
            o.pollable = nullptr;
            return *this;
        }
        inline ~Process()
        {
            pollable = nullptr;
        }
    };

    std::unordered_map<std::string, std::unique_ptr<Process>> m_procs { };
    std::deque<std::pair<const std::string, shed::pause_callback_t>> m_paused { };
    std::vector<actor::Actor*> m_initial_resume { };

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
        , m_waiting { std::move(o.m_waiting) }
        , m_procs { std::move(o.m_procs) }
        , m_paused { std::move(o.m_paused) }
        , m_initial_resume { std::move(o.m_initial_resume) }
    {
    }
    inline MachineGraph& operator=(MachineGraph&& o)
    {
        m_comps = std::move(o.m_comps);
        m_named_comps = std::move(o.m_named_comps);
        m_conns = std::move(o.m_conns);
        m_named_conns = std::move(o.m_named_conns);
        m_waiting = std::move(o.m_waiting);
        m_procs = std::move(o.m_procs);
        m_paused = std::move(o.m_paused);
        m_incidents = std::move(o.m_incidents);
        m_initial_resume = std::move(o.m_initial_resume);
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
        auto proc = std::make_unique<Process>(std::move(act), pollable);
        m_initial_resume.push_back(&proc->actor);
        this->m_procs.emplace(
            std::make_pair(
                with_name,
                std::move(proc)));
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
        if (is_connector(name)) {
            throw std::runtime_error(
                std::format("'{}' connection already exists",
                    name));
        }
        if (!is_component(from))
            throw std::runtime_error(
                std::format("{} from component '{}' does not exist",
                    name, from));
        if (!is_component(to))
            throw std::runtime_error(
                std::format("{} to component '{}' does not exist",
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
        const auto add_incident = [&](Comp* c) {
            if (!m_incidents.contains(c)) {
                m_incidents[c] = { conn.get() };
            } else {
                m_incidents[c].push_back(conn.get());
            }
        };
        add_incident(to_ptr);
        add_incident(from_ptr);
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
        if (name.empty()) {
            throw std::runtime_error("attempted to create a component with no name");
        }
        if (!m_named_comps.empty() && m_named_comps.contains(name)) {
            throw std::runtime_error("component already exists");
        }
        m_named_comps[name] = comp.get();
        m_comps.push_back(comp);
        register_actor(name, comp.get());
        return comp.get();
    }

private:
    inline void remove_connection(const std::string& name, bool skip_incident = false)
    {
        auto* conn = m_named_conns[name];
        if (!skip_incident) {
            if (auto ai = get_incident_to(conn
                        ->get_start()
                        ->get_name());
                ai) {
                incident_t* inc = *ai;
                std::erase_if(*inc, [](auto* c) {
                    return c == c;
                });
            }
            if (auto bi = get_incident_to(conn
                        ->get_end()
                        ->get_name());
                bi) {
                incident_t* inc = *bi;
                std::erase_if(*inc, [](auto* c) {
                    return c == c;
                });
            }
        }
        m_named_conns.erase(name);
        m_conns.erase(
            std::find_if(
                m_conns.begin(),
                m_conns.end(),
                [&](auto& conn) {
                    return conn->get_name() == name;
                }));
    }
    inline void remove_component(const std::string& name)
    {
        auto comp_it = std::find_if(
            m_comps.begin(),
            m_comps.end(),
            [&](auto& comp) {
                return comp->get_name() == name;
            });
        std::shared_ptr<Comp> comp = *comp_it;
        if (auto inc = get_incident_to(name); inc) {
            for (auto* i : **inc) {
                remove_connection(i->get_name(), true);
            }
            m_incidents.erase(comp_it->get());
        }
        m_comps.erase(comp_it);
        m_named_comps.erase(name);
    }

public:
    inline void remove_element(const std::string& name)
    {
        if (m_procs.contains(name))
            m_procs.erase(name);

        if (is_component(name))
            remove_component(name);
        else
            remove_connection(name);
    }

private:
    inline void do_recv(const std::string& pn, std::unique_ptr<Process>& proc)
    {
        if (!proc->awaiting_msg)
            return;
        if (proc->msgq.empty())
            return;
        auto ms = std::move(proc->msgq.front());
        proc->msgq.pop_front();
        if (!exists(ms.sender))
            return;
        // cannot send from comp to comp
        if (is_component(pn) && is_component(ms.sender)) {
            ms.sender_callback(
                std::runtime_error("attempted to message another component directly"));
            return;
        }
        if (is_connector(pn) && is_connector(ms.sender)) {
            ms.sender_callback(
                std::runtime_error("attempted to message another connector directly"));
            return;
        }
        Connection* conn { };
        Component* comp { };
        if (is_connector(pn)) {
            conn = m_named_conns[pn];
            comp = m_named_comps[ms.sender];
        } else {
            conn = m_named_conns[ms.sender];
            comp = m_named_comps[pn];
        }
        if (!conn && !comp)
            return;
        if (conn->get_end() != comp && conn->get_start() != comp) {
            ms.sender_callback(
                std::runtime_error("reciever is not connected to this element"));
            return;
        }
        auto call = std::move(*proc->awaiting_msg);
        proc->awaiting_msg = std::nullopt;
        call(ms.sender, std::move(ms.payload));
        ms.sender_callback(std::nullopt);
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
        for (auto& [pn, proc] : m_procs) {
            do_recv(pn, proc);
            if (proc->paused) {
                auto call = std::move(*proc->paused);
                proc->paused = std::nullopt;
                call();
            }
        }
        if (!m_initial_resume.empty()) {
            for (auto* actor : m_initial_resume) {
                actor->resume();
            }
            m_initial_resume.clear();
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
        // m_paused.push_back(std::make_pair(name, std::move(clb)));
        m_procs[name]->paused = std::move(clb);
    }
    inline virtual void send(
        std::string sender,
        std::string recipent,
        message_t msg,
        shed::send_callback_t c)
    {
        if (m_procs.contains(recipent)) {
            auto& proc = m_procs[recipent];
            proc->msgq.emplace_back(
                sender,
                std::move(msg),
                std::move(c));
        }
    }
    inline virtual void recv(
        std::string who,
        shed::recv_callback_t c)
    {
        m_procs[who]->awaiting_msg = c;
    }
    inline virtual std::optional<std::any> try_recv(
        std::string who)
    {
        auto& proc = m_procs[who];
        std::optional<std::any> msg;
        auto capture_msg = [&](std::string, message_t&& m) {
            msg = m;
        };
        proc->awaiting_msg = capture_msg;
        do_recv(who, proc);
        return msg;
    }
    inline virtual void broadcast(
        std::string sender,
        message_t&& msg,
        shed::broadcast_callback_t clb,
        std::optional<const std::vector<std::string>&> recipents)
    {
        std::optional<
            std::vector<shed::broadcast_fail_t>>
            fails;
        const auto deliver_to = [&](const std::string& proc_name) {
            if (m_procs.contains(proc_name)) {
                auto& proc = m_procs[proc_name];
                auto cpy_msg = message_t(msg);
                proc->msgq.emplace_back(
                    sender,
                    std::move(cpy_msg),
                    [](auto) { });
            } else if (!fails) {
                fails = std::vector<shed::broadcast_fail_t>();
                fails->push_back(
                    std::make_pair(
                        proc_name,
                        std::runtime_error("recipent does not exist")));
            } else {
                fails->push_back(
                    std::make_pair(
                        proc_name,
                        std::runtime_error("recipent does not exist")));
            }
        };
        if (recipents && !recipents->empty()) {
            for (const auto& proc_name : *recipents) {
                deliver_to(proc_name);
            }
        } else if (auto inc = get_incident_to(sender); inc && !(*inc)->empty()) {
            using namespace std::views;
            for (const auto& proc_name : **inc | transform([&](const auto* inc) {
                     return inc->get_name();
                 })) {
                deliver_to(proc_name);
            }
        }
        clb(std::move(fails));
    }
};
}
