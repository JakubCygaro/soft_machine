#ifndef MACHINE_GRAPH_HPP
#define MACHINE_GRAPH_HPP
#include <any>
#include <concepts>
#include <deque>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace machine {
class Component;
class Connection;

using Message = std::any;

class MachineGraph {
    std::list<std::shared_ptr<Component>> components { };
    std::unordered_map<std::string, Component*> named_components { };
    std::list<std::shared_ptr<Connection>> connections { };
    std::unordered_map<std::string, Connection*> named_connections { };

    std::deque<Component*> compq { };
    std::deque<Connection*> connq { };

    std::deque<Message> msgq { };

public:
    template <std::derived_from<Connection> T>
    inline T* create_connection(
        std::string name,
        std::string from,
        std::string to,
        std::shared_ptr<T> conn)
    {
        if (named_connections.contains(name)) {
            throw std::runtime_error("Connection already exists");
        }
        if (!named_components.contains(from))
            throw std::runtime_error("from component does not exist");
        if (!named_components.contains(to))
            throw std::runtime_error("from component does not exist");

        std::shared_ptr<Connection> conn2 = conn;
        named_connections[name] = conn2.get();
        connections.push_back(conn);
        connq.push_back(conn2.get());
        return conn.get();
    }
    template <std::derived_from<Component> T>
    inline T* create_component(std::shared_ptr<T> comp)
    {
        const auto name = comp->get_name();
        if (named_components.contains(name)) {
            throw std::runtime_error("Component already exists");
        }
        std::shared_ptr<Component> comp2 = comp;
        named_components[name] = comp2.get();
        components.push_back(comp);
        compq.push_back(comp2.get());
        return comp.get();
    }

    void poll_all();

    class MachineContext {
        std::deque<Message>* msgq;

    public:
        MachineContext(std::deque<Message>* msgq);
        void send_message(Message&& m);
    };
};
}

#endif
