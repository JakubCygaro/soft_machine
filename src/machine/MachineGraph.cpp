#include "machine/MachineGraph.hpp"
#include <print>
namespace machine {
using ahandle_t = machine::actor::Actor::handle_t;
void MachineGraph::deliver_messages()
{
    for (auto i = m_msgq.size(); i > 0; i--) {
        auto ms = std::move(m_msgq.front());
        m_msgq.pop_front();
        if (m_recipents.contains(ms.recipent)){
            m_recipents[ms.recipent](std::move(ms.payload));
            m_recipents.erase(ms.recipent);
            ms.callback();
        } else {
            m_msgq.push_back(std::move(ms));
        }
    }
}
void MachineGraph::poll_all()
{
    static bool once;
    if (!once) {
        std::println("initializing actors");
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
void MachineGraph::pause(ahandle_t h)
{
    m_scheduled.push_back(h);
}
void MachineGraph::send(
    std::string sender,
    std::string recipent,
    message_t msg,
    send_callback_t c)
{
    m_msgq.push_back(
        MessageSent {
            .sender = sender,
            .recipent = recipent,
            .payload = std::move(msg),
            .callback = c });
}
void MachineGraph::recv(
    std::string who,
    recv_callback_t c)
{
    m_recipents[who] = c;
}
// OneShot<bool>::Read MachineGraph::send_message_req(
//     std::string from,
//     std::string to,
//     Message&& msg)
// {
//     auto [r, w] = OneShot<bool>::create();
//     m_msgq.push_back(MessageRequest {
//         .sender = from,
//         .recipent = to,
//         .payload = std::move(msg),
//         .notify = std::move(w) });
//     return std::move(r);
// }
// OneShot<Message>::Read MachineGraph::recv_message_req(std::string who)
// {
//     auto [r, w] = OneShot<Message>::create();
//     m_msg_recipents.emplace(who, std::move(w));
//     return std::move(r);
// }
}
