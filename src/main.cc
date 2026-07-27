#include "machine/Actor.hpp"
#include "machine/Component.hpp"
#include "machine/Connection.hpp"
#include "machine/MachineGraph.hpp"
#include <any>
#include <optional>
#include <print>
#include <string>

struct Packet {
    std::string sender;
    std::string recipent;
    std::any payload;
};

class SimpleConnection : public machine::Connection {
public:
    SimpleConnection(machine::Component* a, machine::Component* b)
        : machine::Connection(a, b)
    {
    }
    virtual ~SimpleConnection() { }
    virtual machine::actor::Actor
    poll(machine::MachineGraph::MachineContext) override
    {
        co_return;
    }
};

class Sender : public machine::Component {
public:
    Sender(std::string name)
        : machine::Component(name)
    {
    }
    Sender() = delete;
    virtual ~Sender() { }
    virtual machine::actor::Actor
    poll(machine::MachineGraph::MachineContext ctx) override
    {
        std::println("{}", this->name);
        std::flush(std::cout);
        co_await ctx.send("s->r",
            Packet {
                .sender = name,
                .recipent = "recv",
                .payload = "Siema eniu",
            });
        std::println("{} came back from pause ", this->name);
        std::flush(std::cout);
        co_return;
    }
};
class Receiver : public machine::Component {
public:
    Receiver(std::string name)
        : machine::Component(name)
    {
    }
    Receiver() = delete;
    virtual ~Receiver() { }
    virtual machine::actor::Actor
    poll(machine::MachineGraph::MachineContext ctx) override
    {
        co_await ctx.pause();
        co_return;
    }
};
int main(void)
{
    auto m = machine::MachineGraph();
    m.create_component<Sender>("send");
    m.create_component<Receiver>("recv");
    m.create_connection<SimpleConnection>(
        "s->r",
        "send",
        "recv");
    m.poll_all();
    m.poll_all();
}
