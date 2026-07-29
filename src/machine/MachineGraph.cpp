#include "machine/MachineGraph.hpp"
#include <optional>
namespace machine {
using ahandle_t = machine::actor::Actor::handle_t;

MachineGraph::MachineGraph() { }
MachineGraph::MachineGraph(MachineGraph&& o)
    : m_comps { std::move(o.m_comps) }
    , m_named_comps { std::move(o.m_named_comps) }
    , m_conns { std::move(o.m_conns) }
    , m_named_conns { std::move(o.m_named_conns) }
    , m_msgq { std::move(o.m_msgq) }
    , m_waiting { std::move(o.m_waiting) }
    , m_procs { std::move(o.m_procs) }
    , m_scheduled { std::move(o.m_scheduled) }
{
}
MachineGraph& MachineGraph::operator=(MachineGraph&& o)
{
    m_comps = std::move(o.m_comps);
    m_named_comps = std::move(o.m_named_comps);
    m_conns = std::move(o.m_conns);
    m_named_conns = std::move(o.m_named_conns);
    m_msgq = std::move(o.m_msgq);
    m_waiting = std::move(o.m_waiting);
    m_procs = std::move(o.m_procs);
    m_scheduled = std::move(o.m_scheduled);
    return *this;
}
MachineGraph::~MachineGraph()
{
}
bool MachineGraph::is_connector(const std::string& n) const
{
    return m_named_conns.contains(n);
}
bool MachineGraph::is_component(const std::string& n) const
{
    return m_named_comps.contains(n);
}
void MachineGraph::deliver_messages()
{
    for (auto i = m_msgq.size(); i > 0; i--) {
        auto ms = std::move(m_msgq.front());
        m_msgq.pop_front();
        if (!m_waiting.contains(ms.recipent)) {
            m_msgq.push_back(std::move(ms));
            continue;
        }
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
void MachineGraph::poll_all()
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
    decltype(m_scheduled) scheduled = decltype(m_scheduled)(m_scheduled);
    m_scheduled.clear();
    while (!scheduled.empty()) {
        auto h = scheduled.front();
        scheduled.pop_front();
        h.resume();
    }
}
MachineGraph::comp_ref MachineGraph::get_components() const
{
    return this->m_comps;
}
MachineGraph::conn_ref MachineGraph::get_connections() const
{
    return this->m_conns;
}
void MachineGraph::pause(ahandle_t h)
{
    m_scheduled.push_back(h);
}
void MachineGraph::send(
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
void MachineGraph::recv(
    std::string who,
    shed::recv_callback_t c)
{
    m_waiting[who] = c;
}
}
