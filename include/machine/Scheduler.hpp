#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP
#include "machine/Actor.hpp"
#include "machine/Message.hpp"
#include <functional>
#include <string>

namespace machine::shed {
class Scheduler {
public:
    virtual void pause(machine::actor::Actor::handle_t) = 0;
    using send_callback_t = std::function<void(void)>;
    virtual void send(
        std::string sender,
        std::string recipent,
        message_t,
        send_callback_t) = 0;
    using recv_callback_t = std::function<void(message_t&&)>;
    virtual void recv(
        std::string who,
        recv_callback_t) = 0;
};
}

#endif
