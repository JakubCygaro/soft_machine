#include "components/GameGraphElements.hpp"
#include "game/Scene.hpp"
#include "machine/Actor.hpp"
#include "machine/Component.hpp"
#include "machine/Connection.hpp"
#include "machine/MachineGraph.hpp"
#include <any>
#include <iostream>
#include <print>
#include <raylib.h>
#include <string>

struct Packet {
    std::string sender;
    std::string recipent;
    std::any payload;
};

class Passthrough : public components::OConnection {
private:
public:
    Passthrough(machine::Component* a,
        machine::Component* b,
        std::string in,
        std::string out)
        : components::OConnection(a, b, in, out)
    {
    }
    virtual void draw() override
    {
        ::DrawLineEx(m_start_pos, m_end_pos, 5.f, ::BLACK);
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

class Sender : public components::OComponent {
public:
    Sender(std::string name)
        : components::OComponent(name)
    {
        m_bounds = {
            .x = 10,
            .y = 10,
            .width = 10,
            .height = 10,
        };
    }
    Sender() = delete;
    virtual ~Sender() { }
    virtual void draw() override
    {
        ::DrawRectangleRec(m_bounds, ::RED);
        ::DrawTextEx(
            ::GetFontDefault(),
            name.c_str(),
            get_pos(),
            10,
            5,
            ::BLACK);
    }
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
class Receiver : public components::OComponent {
public:
    Receiver(std::string name)
        : components::OComponent(name)
    {
        m_bounds = {
            .x = -10,
            .y = -10,
            .width = 10,
            .height = 10,
        };
    }
    Receiver() = delete;
    virtual ~Receiver() { }
    virtual void draw() override
    {
        ::DrawRectangleRec(m_bounds, ::RED);
        ::DrawTextEx(
            ::GetFontDefault(),
            name.c_str(),
            get_pos(),
            10,
            5,
            ::GREEN);
    }
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
    auto scene = game::GraphScene(std::move(m), { 0, 0, 800, 600 });
    ::InitWindow(800, 600, "Soft Machine");
    ::SetTargetFPS(60);
    while (!::WindowShouldClose()) {
        scene.update();
        ::BeginDrawing();
        scene.draw();
        ::EndDrawing();
    }
    ::CloseWindow();
    m.poll_all();
    // m.poll_all();
    // m.poll_all();
    // m.poll_all();
    // m.poll_all();
}
