#include "machine/MachineGraph.hpp"
namespace machine {

void MachineGraph::deliver_messages()
{
    for (auto i = msgq.size(); i > 0; i--) {
        auto p = std::move(msgq.front());
        msgq.pop_front();

    }
}
void MachineGraph::poll_all()
{
    for (auto i = procs.size(); i > 0; i--) {
        auto p = std::move(procs.front());
        procs.pop_front();
        if (p.actor.await_ready()) {
            std::println("resuming");
            std::flush(std::cout);
            auto res = p.actor.await_resume();
            if (!res)
                throw std::runtime_error("Actor exited");
        }
        procs.push_back(std::move(p));
    }
}
OneShot<bool>::Read MachineGraph::send_message_req(
    std::string to,
    Message&& msg)
{
    auto [r, w] = OneShot<bool>::create();
    msgq.push_back(MessageRequest {
        .recipent = to,
        .payload = std::move(msg),
        .notify = std::move(w) });
    return std::move(r);
}
}
