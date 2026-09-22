#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP
#include "machine/Message.hpp"
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>

namespace machine::shed {
using pause_callback_t = std::function<void()>;
using send_callback_t = std::function<void(std::optional<std::runtime_error>)>;
using recv_callback_t = std::function<void(std::string, message_t&&)>;
class Scheduler {
public:
    virtual void pause(const std::string& name, pause_callback_t) = 0;
    virtual void send(
        std::string sender,
        std::string recipent,
        message_t,
        send_callback_t) = 0;
    // the sender of the message and the message
    virtual void recv(
        std::string who,
        recv_callback_t) = 0;
};
}

#endif
