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

struct Message {
    std::any payload { };
    std::string sender { };
    std::string receiver { };
};

class MachineContext {
    std::deque<Message>* msgq;

public:
    MachineContext(std::deque<Message>* msgq);
    void send_message(Message&& m);
};

class Component;
class Connection;

class Trigger {
public:
    virtual void check_trigger() = 0;
};

class Component {
private:
    std::unordered_map<std::string, std::weak_ptr<Connection>> named_connections { };

protected:
    const std::string name { };

public:
    std::deque<Message> msgq { };

    explicit Component(std::string name);
    virtual ~Component();
    virtual void add_connection(std::weak_ptr<Connection> conn, std::string name);
    const std::string& get_name() const;
    virtual void poll(MachineContext ctx) = 0;
};

class Connection {
private:
    Component *start { }, *end { };

public:
    std::deque<Message> msgq { };

    explicit Connection(Component* start, Component* end);
    virtual ~Connection();
    const Component* get_start() const;
    const Component* get_end() const;
    virtual void poll(MachineContext ctx) = 0;
};

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
    inline T* create_connection(std::string name, std::shared_ptr<T> conn)
    {
        if (named_connections.contains(name)) {
            throw std::runtime_error("Connection already exists");
        }
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
};
#endif
