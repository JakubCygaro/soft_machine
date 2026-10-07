#include "machine/MachineContext.hpp"
#include "machine/Scheduler.hpp"
#include <cstdint>
#include <utility>
namespace machine {
MachineContext::MachineContext(std::uint32_t id_of_this, shed::Scheduler* s)
    : m_id_of_this { id_of_this }
    , m_sched { s }
{
}
MachineContext::Pause MachineContext::pause() const
{
    return Pause(this->m_sched, this->m_id_of_this);
}

MachineContext::Send MachineContext::send(std::string recipent, message_t&& msg)
{
    return Send(this->m_sched, this->m_id_of_this, recipent, std::move(msg));
}
MachineContext::Recv MachineContext::recv()
{
    return Recv(this->m_sched, this->m_id_of_this);
}
MachineContext::SendRecv MachineContext::send_recv(std::string recipent, message_t&& msg)
{
    return SendRecv(this->m_sched, this->m_id_of_this, recipent, std::move(msg));
}
std::optional<message_t> MachineContext::try_recv()
{
    return this->m_sched->try_recv(m_id_of_this);
}

MachineContext::Broadcast MachineContext::broadcast(
     message_t&&msg, std::optional<std::vector<std::string>> rcv)
{
    return Broadcast(m_sched, m_id_of_this, rcv, std::move(msg));
}
}
