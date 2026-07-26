#include <any>
#include <concepts>
#include <deque>
#include <list>
#include <memory>
#include <print>
#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_map>

struct Message {
    std::any payload { };
    std::string sender { };
    std::string receiver { };
};
struct TransferMessage {
    std::any payload { };
    std::string destination { };
};

class MachineContext {
    std::deque<Message>* msgq;

public:
    MachineContext(
        std::deque<Message>* msgq)
        : msgq { msgq }

    {
    }
    void send_message(Message&& m)
    {
        msgq->push_back(std::move(m));
    }
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

    explicit Component(std::string name)
        : name { name }
    {
    }
    virtual void add_connection(std::weak_ptr<Connection> conn, std::string name)
    {
        named_connections[name] = conn;
    }
    virtual void poll(MachineContext ctx) = 0;
    virtual ~Component() { };
    const std::string& get_name() const
    {
        return name;
    }
};

class Connection {
private:
    Component *start { }, *end { };

public:
    std::deque<Message> msgq { };

    explicit Connection(Component* start, Component* end)
        : start { start }
        , end { end }
    {
    }
    const Component* get_start() const
    {
        return this->start;
    }
    const Component* get_end() const
    {
        return this->end;
    }
    virtual void poll(MachineContext ctx) = 0;
    virtual ~Connection() { };
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
    T* create_connection(std::string name, std::shared_ptr<T> conn)
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
    T* create_component(std::shared_ptr<T> comp)
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
    void poll_all()
    {
        while (!msgq.empty()) {
            auto m = msgq.front();
            msgq.pop_front();
            if (this->named_components.contains(m.receiver)) {
                named_components[m.receiver]->msgq.push_back(std::move(m));
            } else if (this->named_connections.contains(m.receiver)) {
                named_connections[m.receiver]->msgq.push_back(std::move(m));
            }
        }

        std::deque<Component*> comp_polled { };
        std::string name("bullshit");
        while (!compq.empty()) {
            auto c = compq.front();
            compq.pop_front();
            c->poll(MachineContext(&this->msgq));
            comp_polled.push_back(c);
        }
        compq = comp_polled;
        std::deque<Connection*> conn_polled { };
        while (!connq.empty()) {
            auto c = connq.front();
            connq.pop_front();
            c->poll(MachineContext(&this->msgq));
            conn_polled.push_back(c);
        }
        connq = conn_polled;
    }
};

class SimpleConnection : public Connection {
public:
    SimpleConnection(Component* start, Component* end)
        : Connection(start, end)
    {
    }
    void poll(MachineContext ctx) override
    {
        while (!msgq.empty()) {
            auto m = msgq.front();
            msgq.pop_front();
            auto p = std::any_cast<TransferMessage>(m.payload);
            ctx.send_message(Message {
                .payload = p.payload,
                .sender = m.sender,
                .receiver = p.destination,
            });
        }
    }
};

class Sender : public Component {
private:
    bool done { false };

public:
    Sender(std::string name)
        : Component(name)
    {
    }
    void poll(MachineContext ctx) override
    {
        if (!done) {
            std::println("Sending");
            ctx.send_message(Message {
                .payload = std::any(TransferMessage {
                    .payload = std::string("Hello there"),
                    .destination = "receiver" }),
                .sender = name,
                .receiver = "s->r",
            });
            done = true;
        }
    }
};
class Receiver : public Component {
private:
    bool done { false };

public:
    Receiver(std::string name)
        : Component(name)
    {
    }
    void poll(MachineContext ctx) override
    {
        if (!done && !msgq.empty()) {
            auto m = msgq.front();
            msgq.pop_front();
            auto p = std::any_cast<std::string>(m.payload);
            std::println("{}", p);
            done = true;
        }
    }
};

int main(void)
{
    auto m = MachineGraph();
    auto s = m.create_component(std::make_shared<Sender>("sender"));
    auto r = m.create_component(std::make_shared<Receiver>("receiver"));
    m.create_connection("s->r", std::make_shared<SimpleConnection>(s, r));
    m.poll_all();
    m.poll_all();
    m.poll_all();
}
