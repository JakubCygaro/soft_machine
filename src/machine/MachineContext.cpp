#include "machine/MachineGraph.hpp"
MachineContext::MachineContext(
    std::deque<Message>* msgq)
    : msgq { msgq }

{
}
void MachineContext::send_message(Message&& m)
{
    msgq->push_back(std::move(m));
}
