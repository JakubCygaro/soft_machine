#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP
#include "machine/Message.hpp"
#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>

namespace machine::shed {
using pause_callback_t = std::function<void()>;
using send_callback_t = std::function<void(std::optional<std::runtime_error>)>;
using broadcast_fail_t = std::pair<std::string, std::runtime_error>;
using broadcast_callback_t = std::function<
    void(std::optional<
        std::vector<broadcast_fail_t>>&&)>;
using recv_callback_t = std::function<void(std::string, message_t&&)>;
class Scheduler {
public:
    virtual void pause(const std::uint32_t& id, pause_callback_t) = 0;
    virtual void send(
        const std::uint32_t& self,
        std::string recipent,
        message_t,
        send_callback_t) = 0;
    virtual void broadcast(
        const std::uint32_t& self,
        message_t&& msg,
        shed::broadcast_callback_t clb,
        std::optional<const std::vector<std::string>&> recipents) = 0;
    // the sender of the message and the message
    virtual void recv(
        const std::uint32_t& self,
        recv_callback_t) = 0;
    virtual std::optional<message_t> try_recv(
        const std::uint32_t& self) = 0;
};
}

#endif
