#include "machine/Actor.hpp"
#include "machine/Component.hpp"
#include "machine/Connection.hpp"
#include "machine/MachineGraph.hpp"
#include <any>
#include <iostream>
#include <print>
#include <string>

struct Packet {
    std::string sender;
    std::string recipent;
    std::any payload;
};

class Passthrough : public machine::Connection {
private:
public:
    Passthrough(machine::Component* a,
        machine::Component* b,
        std::string in,
        std::string out)
        : machine::Connection(a, b, in, out)
    {
    }
    virtual ~Passthrough() { }
    virtual machine::actor::Actor
    poll(machine::Mctx ctx) override
    {
        while (true) {
            auto [_, m] = co_await ctx.recv();
            co_await ctx.send(out, std::move(m));
        }
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
    poll(machine::Mctx ctx) override
    {
        std::println("{}", this->name);
        std::flush(std::cout);
        co_await ctx.send("s->r",
            std::string("Siema eniu"));
        std::println("{} came back from pause, now will wait for response",
            this->name);
        std::flush(std::cout);
        auto [_, resp] = co_await ctx.recv();
        std::println("{} got response {}",
            this->name,
            std::any_cast<std::string>(resp));
        std::flush(std::cout);
        while (true)
            co_await ctx.pause();
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
    poll(machine::Mctx ctx) override
    {
        auto [_, m] = co_await ctx.recv();
        auto s = std::any_cast<std::string>(m);
        std::println("{} got message {} ", this->name, s);
        std::println("{} sending response", this->name);
        co_await ctx.send("r->s", std::string("Eniu siema"));
        std::flush(std::cout);
        while (true)
            co_await ctx.pause();
    }
};
int main(void)
{
    auto m = machine::MachineGraph();
    auto s = m.create_component<Sender>("send");
    auto r = m.create_component<Receiver>("recv");
    m.create_connection<Passthrough>(
        "s->r",
        s->get_name(),
        r->get_name());
    m.create_connection<Passthrough>(
        "r->s",
        r->get_name(),
        s->get_name());
    m.poll_all();
    m.poll_all();
    m.poll_all();
    m.poll_all();
    m.poll_all();
}
