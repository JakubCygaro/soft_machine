#include "machine/MachineContext.hpp"
#include "machine/Scheduler.hpp"
#include <utility>
namespace machine {
MachineContext::MachineContext(std::string name_of_this, shed::Scheduler* s)
    : m_name_of_this { name_of_this }
    , m_sched { s }
{
}
MachineContext::Pause MachineContext::pause() const
{
    return Pause(this->m_sched, this->m_name_of_this);
}

MachineContext::Send MachineContext::send(std::string recipent, message_t&& msg)
{
    return Send(this->m_sched, this->m_name_of_this, recipent, std::move(msg));
}
MachineContext::Recv MachineContext::recv()
{
    return Recv(this->m_sched, this->m_name_of_this);
}
MachineContext::SendRecv MachineContext::send_recv(std::string rcv, message_t&& msg)
{
    return SendRecv(this->m_sched, rcv, std::move(msg));
}
std::optional<message_t> MachineContext::try_recv()
{
    return this->m_sched->try_recv(m_name_of_this);
}

MachineContext::Broadcast MachineContext::broadcast(
    std::vector<std::string> rcv, message_t&& msg)
{
    return Broadcast(m_sched, m_name_of_this, rcv, std::move(msg));
}
}
