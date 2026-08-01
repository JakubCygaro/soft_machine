#ifndef MACHINE_CONTEXT_HPP
#define MACHINE_CONTEXT_HPP
#include "machine/Actor.hpp"
#include "machine/Message.hpp"
#include "common/Result.hpp"
#include "machine/Scheduler.hpp"
#include <any>
#include <stdexcept>
#include <string>
#include <utility>
namespace machine {

class MachineContext {
    friend class MachineGraph;
    using shd = shed::Scheduler;

private:
    std::string m_name_of_this { };
    shed::Scheduler* m_sched { };

    MachineContext(std::string name_of_this, shed::Scheduler* s);

public:
    struct Pause {
        friend class MachineContext;

    private:
        shd* m_s { };
        inline Pause(shd* s)
            : m_s { s }
        {
        }
        inline Pause(const Pause&) = delete;
        inline Pause& operator=(const Pause&) = delete;
        inline Pause(Pause&& o)
            : m_s { o.m_s }
        {
            o.m_s = nullptr;
        }
        inline Pause& operator=(Pause&& o)
        {
            m_s = o.m_s;
            o.m_s = nullptr;
            return *this;
        }

    public:
        inline bool await_ready() { return false; }
        inline void await_suspend(actor::Actor::handle_t h)
        {
            this->m_s->pause(h);
        }
        inline void await_resume() { }
    };
    Pause pause() const;

    struct Send {
        friend class MachineContext;

    private:
        shd* m_s { };
        std::string m_sender { }, m_reciever { };
        message_t m_msg;
        result_t<std::runtime_error, Unit> m_ret { Unit { } };
        inline Send(
            shd* s,
            std::string snd,
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
                    m_ret = *err;
                m_s->pause(h);
            };
            m_s->send(
                m_sender,
                m_reciever,
                std::move(m_msg),
                on_send);
            m_msg = nullptr;
        }
        inline result_t<std::runtime_error, Unit> await_resume() { return m_ret; }
    };
    Send send(std::string recipent, message_t&& msg);
    struct Recv {
        friend class MachineContext;

    private:
        shd* m_s { };
        std::string m_reciever { };
        std::string m_sender;
        message_t m_msg;
        inline Recv(
            shd* s,
            std::string rcv)
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
                m_s->pause(h);
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
};
using Mctx = MachineContext;
}
#endif
