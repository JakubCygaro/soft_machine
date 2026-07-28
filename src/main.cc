#include "machine/Actor.hpp"
#include "machine/Component.hpp"
#include "machine/Connection.hpp"
#include "machine/MachineGraph.hpp"
#include <any>
#include <print>
#include <iostream>
#include <print>
#include <string>

struct Packet {
    std::string sender;
    std::string recipent;
    std::any payload;
};

class Passthrough : public machine::Connection {
public:
    Passthrough(machine::Component* a, machine::Component* b)
        : machine::Connection(a, b)
    {
    }
    virtual ~Passthrough() { }
    virtual machine::actor::Actor
    poll(machine::Mctx ctx) override
    {
        while (true) {
            auto m = co_await ctx.recv();
            co_await ctx.send("recv", std::move(m));
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
        std::println("{} came back from pause ", this->name);
        std::flush(std::cout);
        while(true) co_await ctx.pause();
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
        auto m = co_await ctx.recv();
        auto s = std::any_cast<std::string>(m);
        std::println("{} got message {} ", this->name, s);
        std::flush(std::cout);
        while(true) co_await ctx.pause();
    }
};
int main(void)
{
    auto m = machine::MachineGraph();
    m.create_component<Sender>("send");
    m.create_component<Receiver>("recv");
    m.create_connection<Passthrough>(
        "s->r",
        "send",
        "recv");
    m.poll_all();
    m.poll_all();
    m.poll_all();
    m.poll_all();
    m.poll_all();
    // while(true){
    //     std::string l_;
    //     std::getline(std::cin, l_);
    // }
}
