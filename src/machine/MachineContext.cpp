#include "machine/MachineContext.hpp"
#include "machine/Scheduler.hpp"
#include <any>
#include <utility>
namespace machine {
MachineContext::MachineContext(std::string name_of_this, shed::Scheduler* s)
    : m_name_of_this { name_of_this }
    , m_sched { s }
{
}
MachineContext::Pause MachineContext::pause() const
{
    return Pause(this->m_sched);
}

MachineContext::Send MachineContext::send(std::string recipent, message_t&& msg)
{
    return Send(this->m_sched, this->m_name_of_this, recipent, std::move(msg));
}
MachineContext::Recv MachineContext::recv()
{
    return Recv(this->m_sched, this->m_name_of_this);
}
}
