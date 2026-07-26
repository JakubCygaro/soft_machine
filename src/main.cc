#include <any>
#include <deque>
#include <memory>
#include <print>
#include <string>
#include "machine/MachineGraph.hpp"

struct TransferMessage {
    std::any payload { };
    std::string destination { };
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
