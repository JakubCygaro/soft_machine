#include "machine/MachineGraph.hpp"
namespace machine {
MachineGraph::MachineContext::MachineContext(
    std::string name_of_this,
    send_msg_fn_t send_msg)
    : name_of_this { name_of_this }
    , send_msg_fn { send_msg }

{
}
// void MachineGraph::MachineContext::send_message(Message&& m)
// {
//     msgq->push_back(std::move(m));
// }
MachineGraph::MachineContext::Pause MachineGraph::MachineContext::pause()
{
    return MachineGraph::MachineContext::Pause { };
}

MachineGraph::MachineContext::Send
MachineGraph::MachineContext::send(
    std::string to,
    Message&& m)
{
    auto r = this->send_msg_fn(name_of_this, to, std::move(m));
    return MachineGraph::MachineContext::Send {
        to,
        std::move(m),
        std::move(r),
    };
}
}
