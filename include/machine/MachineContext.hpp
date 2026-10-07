#ifndef MACHINE_CONTEXT_HPP
#define MACHINE_CONTEXT_HPP
#include "common/Result.hpp"
#include "machine/Actor.hpp"
#include "machine/Message.hpp"
#include "machine/Preamble.hpp"
#include "machine/Scheduler.hpp"
#include <any>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
namespace machine {

class MachineContext {
    // class Component;
    // class Connection;
    using shd = shed::Scheduler;

    template <
        std::derived_from<Component> Comp,
        std::derived_from<Connection> Conn>
    friend class MachineGraph;

private:
    std::uint32_t m_id_of_this { };
    shed::Scheduler* m_sched { };

    MachineContext(std::uint32_t id_of_this, shed::Scheduler* s);

public:
    struct Pause {
        friend class MachineContext;

    private:
        shd* m_s { };
        std::uint32_t name;
        inline Pause(shd* s, std::uint32_t name)
            : m_s { s }
            , name { name }
        {
        }
        inline Pause(const Pause&) = delete;
        inline Pause& operator=(const Pause&) = delete;
        inline Pause(Pause&& o, std::uint32_t name)
            : m_s { o.m_s }
            , name { name }
        {
            o.m_s = nullptr;
        }
        inline Pause& operator=(Pause&& o)
        {
            m_s = o.m_s;
            name = o.name;
            o.m_s = nullptr;
            return *this;
        }

    public:
        inline bool await_ready() { return false; }
        inline void await_suspend(actor::Actor::handle_t h)
        {
            this->m_s->pause(name, [=]() {
                h.resume();
            });
        }
        inline void await_resume() { }
    };
    Pause pause() const;

    struct Send {
        friend class MachineContext;

    private:
        shd* m_s { };
        std::uint32_t m_sender { };
        std::string m_reciever { };
        message_t m_msg;
        Result<std::runtime_error, Unit> m_ret { Unit { } };
        inline Send(
            shd* s,
            std::uint32_t snd,
            std::string rcv,
            message_t&& msg)
            : m_s { s }
            , m_sender { snd }
            , m_reciever { rcv }
            , m_msg { msg }
        {
        }
        inline Send(const Send&) = delete;
        inline Send& operator=(const Send&) = delete;
        inline Send(Send&& o)
            : m_s { o.m_s }
            , m_sender { o.m_sender }
            , m_reciever { o.m_reciever }
            , m_msg { std::move(o.m_msg) }
            , m_ret { std::move(o.m_ret) }
        {
            o.m_s = nullptr;
            o.m_msg = nullptr;
        }
        inline Send& operator=(Send&& o)
        {
            m_s = o.m_s;
            m_sender = o.m_sender;
            m_reciever = o.m_reciever;
            m_msg = o.m_msg;
            m_ret = std::move(o.m_ret);
            o.m_s = nullptr;
            o.m_msg = nullptr;
            return *this;
        }

    public:
        inline bool await_ready() { return false; }
        inline void await_suspend(actor::Actor::handle_t h)
        {
            const auto on_send = [h, this](auto err) {
                if (err)
                    m_ret = decltype(m_ret)::err(std::move(*err));
                h.resume();
                // m_s->pause(h);
            };
            m_s->send(
                m_sender,
                m_reciever,
                std::move(m_msg),
                on_send);
            m_msg = nullptr;
        }
        inline Result<std::runtime_error, Unit> await_resume() { return m_ret; }
    };
    Send send(std::string recipent, message_t&& msg);
    struct Recv {
        friend class MachineContext;

    private:
        shd* m_s { };
        std::uint32_t m_reciever { };
        std::string m_sender;
        message_t m_msg;
        inline Recv(
            shd* s,
            std::uint32_t rcv)
            : m_s { s }
            , m_reciever { rcv }
            , m_msg { std::make_any<message_t>(nullptr) }
        {
        }
        inline Recv(const Recv&) = delete;
        inline Recv& operator=(const Recv&) = delete;
        inline Recv(Recv&& o)
            : m_s { o.m_s }
            , m_reciever { o.m_reciever }
            , m_msg { std::move(o.m_msg) }
        {
            o.m_s = nullptr;
            o.m_msg = nullptr;
        }
        inline Recv& operator=(Recv&& o)
        {
            m_s = o.m_s;
            m_reciever = o.m_reciever;
            m_msg = o.m_msg;
            o.m_s = nullptr;
            o.m_msg = nullptr;
            return *this;
        }

    public:
        inline bool await_ready() { return false; }
        inline void await_suspend(actor::Actor::handle_t h)
        {
            const auto on_recv = [h, this](std::string snd, message_t&& msg) {
                this->m_msg = std::move(msg);
                this->m_sender = snd;
                h.resume();
                // m_s->pause(h);
            };
            m_s->recv(
                m_reciever,
                on_recv);
        }
        inline std::tuple<std::string, message_t> await_resume()
        {
            return std::make_tuple(m_sender, m_msg);
        }
    };
    Recv recv();
    struct SendRecv {
        friend class MachineContext;

    private:
        shd* m_s { };
        std::uint32_t m_sender_reciever { };
        std::string m_recipent;
        std::string m_responder;
        message_t m_msg;
        Result<std::runtime_error, message_t> m_ret { Unit { } };
        inline SendRecv(
            shd* s,
            std::uint32_t rcv,
            std::string recipent,
            message_t&& msg)
            : m_s { s }
            , m_sender_reciever { rcv }
            , m_recipent { recipent }
            , m_msg { msg }
            , m_ret { std::make_any<message_t>(nullptr) }
        {
        }
        inline SendRecv(const SendRecv&) = delete;
        inline SendRecv& operator=(const SendRecv&) = delete;
        inline SendRecv(SendRecv&& o)
            : m_s { o.m_s }
            , m_sender_reciever { o.m_sender_reciever }
            , m_recipent { std::move(o.m_recipent) }
            , m_msg { std::move(o.m_msg) }
            , m_ret { std::move(o.m_ret) }
        {
            o.m_s = nullptr;
            o.m_ret = { nullptr };
            o.m_msg = nullptr;
        }
        inline SendRecv& operator=(SendRecv&& o)
        {
            m_s = o.m_s;
            m_sender_reciever = o.m_sender_reciever;
            m_recipent = o.m_recipent;
            m_ret = std::move(o.m_ret);
            m_msg = std::move(o.m_msg);
            o.m_s = nullptr;
            o.m_ret = { nullptr };
            o.m_msg = nullptr;
            return *this;
        }

    public:
        inline bool await_ready() { return false; }
        inline void await_suspend(actor::Actor::handle_t h)
        {
            const auto on_recv = [h, this](std::string snd, message_t&& msg) {
                this->m_ret = { std::move(msg) };
                this->m_responder = snd;
                h.resume();
            };
            const auto on_send = [&, h, this](auto err) {
                if (err) {
                    m_ret = decltype(m_ret)::err(std::move(*err));
                    h.resume();
                } else {
                    m_s->recv(
                        m_sender_reciever,
                        on_recv);
                }
            };
            m_s->send(
                m_sender_reciever,
                m_recipent,
                std::move(m_msg),
                on_send);
            m_msg = nullptr;
        }
        inline std::tuple<std::string, message_t> await_resume()
        {
            return std::make_tuple(m_responder, m_msg);
        }
    };
    SendRecv send_recv(std::string recipent, message_t&& msg);
    std::optional<message_t> try_recv();

    struct Broadcast {
        friend class MachineContext;

    private:
        shd* m_s { };
        std::uint32_t m_sender { };
        std::optional<std::vector<std::string>> m_recievers { };
        message_t m_msg;
        Result<std::vector<shed::broadcast_fail_t>, Unit> m_ret { Unit { } };
        inline Broadcast(
            shd* s,
            std::uint32_t snd,
            std::optional<std::vector<std::string>> rcv,
            message_t&& msg)
            : m_s { s }
            , m_sender { snd }
            , m_recievers { rcv }
            , m_msg { msg }
        {
        }
        inline Broadcast(const Broadcast&) = delete;
        inline Broadcast& operator=(const Broadcast&) = delete;
        inline Broadcast(Broadcast&& o)
            : m_s { o.m_s }
            , m_sender { o.m_sender }
            , m_recievers { o.m_recievers }
            , m_msg { std::move(o.m_msg) }
            , m_ret { std::move(o.m_ret) }
        {
            o.m_s = nullptr;
            o.m_msg = nullptr;
        }
        inline Broadcast& operator=(Broadcast&& o)
        {
            m_s = o.m_s;
            m_sender = o.m_sender;
            m_recievers = o.m_recievers;
            m_msg = o.m_msg;
            m_ret = std::move(o.m_ret);
            o.m_s = nullptr;
            o.m_msg = nullptr;
            return *this;
        }

    public:
        inline bool await_ready() { return false; }
        inline void await_suspend(actor::Actor::handle_t h)
        {
            const auto on_broadcast = [h, this](
                                          std::optional<
                                              std::vector<
                                                  shed::broadcast_fail_t>>&& bf) {
                if (bf) {
                    m_ret = decltype(m_ret)::err(std::move(*bf));
                }
                h.resume();
            };
            m_s->broadcast(
                m_sender,
                std::move(m_msg),
                on_broadcast,
                m_recievers);
            m_recievers = std::nullopt;
            m_msg = nullptr;
        }
        inline Result<std::vector<shed::broadcast_fail_t>, Unit> await_resume()
        {
            return m_ret;
        }
    };
    Broadcast broadcast(
        message_t&& msg, std::optional<std::vector<std::string>> rcv = std::nullopt);
};
using Mctx = MachineContext;
}
#endif
