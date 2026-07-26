#include "machine/MachineGraph.hpp"
namespace machine {
MachineGraph::MachineContext::MachineContext(
    std::deque<Message>* msgq)
    : msgq { msgq }

{
}
void MachineGraph::MachineContext::send_message(Message&& m)
{
    msgq->push_back(std::move(m));
}
}
