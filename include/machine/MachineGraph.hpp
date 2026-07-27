#ifndef MACHINE_GRAPH_HPP
#define MACHINE_GRAPH_HPP
#include "machine/Actor.hpp"
#include <any>
#include <concepts>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <ostream>
#include <print>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace machine {
class Component;
class Connection;

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

    std::deque<Message> msgq { };

    struct Process {
        actor::Actor actor;
    };

    std::deque<Process> procs { };

public:
    class MachineContext {
        std::deque<Message>* msgq;

    public:
        MachineContext(std::deque<Message>* msgq);
        void send_message(Message&& m);
    };

private:
    template <Pollable<MachineContext> T>
    inline void register_actor(T* pollable)
    {
        MachineContext mctx(&this->msgq);
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
        register_actor(conn.get());
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
        // std::shared_ptr<Component> comp2 = comp;
        named_components[name] = comp.get();
        components.push_back(comp);
        register_actor(comp.get());
        return comp.get();
    }

    void poll_all();
};
}

#endif
