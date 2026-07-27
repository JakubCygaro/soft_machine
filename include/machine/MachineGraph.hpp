#ifndef MACHINE_GRAPH_HPP
#define MACHINE_GRAPH_HPP
#include "machine/Actor.hpp"
#include "machine/Task.hpp"
#include <any>
#include <atomic>
#include <concepts>
#include <coroutine>
#include <deque>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <ostream>
#include <print>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>

namespace machine {
class Component;
class Connection;
class Awaitable;

template <std::movable T>
class OneShot {
    using data_t = std::shared_ptr<
        std::atomic<
            std::optional<T>>>;
    OneShot() = delete;

public:
    class Read;
    class Write;
    inline static std::pair<Read, Write> create()
    {
        auto v = std::make_shared<std::atomic<std::optional<T>>>;
        return { Read(v), Write(v) };
    }
    class Read {
        data_t val;
        inline explicit Read(data_t val);
        friend std::pair<Read, Write> create();
        Read(const Read&) = delete;
        Read& operator=(const Read&) = delete;

    public:
        inline Read(Read&& other)
            : val { other.val }
        {
            other.val = nullptr;
        }
        inline Read& operator=(Read&& other)
        {
            this->val = other.val;
            other.val = nullptr;
            return *this;
        }

    public:
        inline T recv() const
        {
            val->wait(std::nullopt);
            return *val->load();
        }
    };
    class Write {
        data_t val;
        inline explicit Write(data_t val);
        friend std::pair<Read, Write> create();
        Write(const Write&) = delete;
        Write& operator=(const Write&) = delete;

    public:
        inline Write(Write&& other)
            : val { other.val }
        {
            other.val = nullptr;
        }
        inline Write& operator=(Write&& other)
        {
            this->val = other.val;
            other.val = nullptr;
            return *this;
        }
        inline void send(T&& v)
        {
            auto inside = val->load();
            if (inside.has_value()) {
                throw std::runtime_error("Already written");
            }
            val->store(std::move(v));
        }
    };
};

using Message = std::any;

class MachineContext;

template <typename T, typename... Args>
concept Pollable = requires(T* t, Args... args) {
    { t->poll(args...) } -> std::same_as<actor::Actor>;
};

class MachineGraph {
private:
    std::list<std::shared_ptr<Component>> components { };
    std::unordered_map<std::string, Component*> named_components { };
    std::list<std::shared_ptr<Connection>> connections { };
    std::unordered_map<std::string, Connection*> named_connections { };

    std::deque<Component*> compq { };
    std::deque<Connection*> connq { };

    struct MessageRequest {
        std::string sender;
        std::string recipent;
        Message payload;
        OneShot<bool>::Write notify;
    };
    std::deque<MessageRequest> msgq { };

    struct Process {
        actor::Actor actor;
    };

    std::deque<Process> procs { };

public:
    class MachineContext {
    public:
        using send_msg_fn_t = std::function<
            OneShot<bool>::Read(std::string, std::string, Message&&)>;

    private:
        std::string name_of_this { };
        send_msg_fn_t send_msg_fn;

    public:
        struct Pause;
        Pause pause();
        struct Pause {
        private:
            inline explicit Pause() { }
            friend Pause MachineContext::pause();

        public:
            inline bool await_ready() const { return false; }
            inline void await_suspend(std::coroutine_handle<> h) const
            {
                (void)h;
            }
            inline void await_resume() const { }
        };
        struct Send;
        Send send(std::string to, Message&& m);
        struct Send {
        private:
            std::string recipent { };
            Message msg { };
            OneShot<bool>::Read notify;
            inline explicit Send(
                std::string r,
                Message&& m,
                OneShot<bool>::Read&& notify)
                : recipent { r }
                , msg { std::move(m) }
                , notify { std::move(notify) }

            {
            }
            Send(const Send&) = delete;
            Send operator=(const Send&) = delete;
            friend Send MachineContext::send(std::string, Message&&);

        public:
            inline bool await_ready() const { return false; }
            inline void await_suspend(std::coroutine_handle<> h) const
            {
                std::thread([this, h]() {
                    notify.recv();
                    h.resume();
                }).detach();
            }
            inline void await_resume() const { }
        };

    public:
        MachineContext(
            std::string name_of_this,
            send_msg_fn_t send_msg);
    };

private:
    OneShot<bool>::Read send_message_req(
        std::string from,
        std::string to,
        Message&& msg);
    template <Pollable<MachineContext> T>
    inline void register_actor(std::string with_name, T* pollable)
    {
        using namespace std::placeholders;
        MachineContext mctx(with_name,
            std::bind(&MachineGraph::send_message_req, this, _1, _2, _3));
        std::println("poll");
        std::flush(std::cout);
        auto act = pollable->poll(mctx);
        this->procs.push_back(
            Process {
                .actor = std::move(act) });
    }

public:
    template <std::derived_from<Connection> T, typename... Args>
    inline T* create_connection(
        std::string name,
        std::string from,
        std::string to,
        Args&&... ctor_args)
    {
        if (named_connections.contains(name)) {
            throw std::runtime_error("Connection already exists");
        }
        if (!named_components.contains(from))
            throw std::runtime_error("from component does not exist");
        if (!named_components.contains(to))
            throw std::runtime_error("to component does not exist");

        auto* from_ptr = named_components[from];
        auto* to_ptr = named_components[to];
        std::shared_ptr<T> conn = nullptr;
        if constexpr (sizeof...(ctor_args) > 0) {
            conn = std::make_shared<T>(from_ptr, to_ptr, std::forward<Args>(ctor_args)...);
        } else {
            conn = std::make_shared<T>(from_ptr, to_ptr);
        }
        named_connections[name] = conn.get();
        connections.push_back(conn);
        register_actor(name, conn.get());
        return conn.get();
    }
    template <std::derived_from<Component> T, typename... Args>
    inline T* create_component(Args&&... ctor_args)
    {
        std::shared_ptr<T> comp = nullptr;
        if constexpr (sizeof...(ctor_args) > 0) {
            comp = std::make_shared<T>(std::forward<Args>(ctor_args)...);
        } else {
            comp = std::make_shared<T>();
        }
        const auto name = comp->get_name();
        if (named_components.contains(name)) {
            throw std::runtime_error("Component already exists");
        }
        named_components[name] = comp.get();
        components.push_back(comp);
        register_actor(name, comp.get());
        return comp.get();
    }

    void poll_all();

private:
    void deliver_messages();
};
}

#endif
