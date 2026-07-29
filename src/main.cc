#include "components/GameGraphElements.hpp"
#include "game/Scene.hpp"
#include "machine/Actor.hpp"
#include "machine/Component.hpp"
#include "machine/Connection.hpp"
#include "machine/MachineGraph.hpp"
#include <any>
#include <iostream>
#include <memory>
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
        ::DrawLineBezier(m_start_pos, m_end_pos, 5.f, ::BLACK);
    }
    virtual ~Passthrough() { }
    virtual machine::actor::Actor
    poll(machine::Mctx ctx) override
    {
        std::println("passt entered");
        while (true) {
            auto [s, m] = co_await ctx.recv();
            if (s == in)
                co_await ctx.send(out, std::move(m));
            else if (s == out)
                co_await ctx.send(in, std::move(m));
        }
    }
};

class Sender : public components::OComponent {
public:
    Sender(std::string name)
        : components::OComponent(name)
    {
        m_bounds = {
            .x = 150,
            .y = 150,
            .width = 100,
            .height = 100,
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
        std::println("{} entered", this->name);
        std::flush(std::cout);
        co_await ctx.send("passt",
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
            .x = -50,
            .y = -50,
            .width = 100,
            .height = 100,
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
        std::println("{} entered", this->name);
        auto [_, m] = co_await ctx.recv();
        auto s = std::any_cast<std::string>(m);
        std::println("{} got message {} ", this->name, s);
        std::println("{} sending response", this->name);
        co_await ctx.send("passt", std::string("Eniu siema"));
        std::flush(std::cout);
        while (true)
            co_await ctx.pause();
    }
};
int main(void)
{
    auto m = std::unique_ptr<machine::MachineGraph>(
        machine::MachineGraph::create());
    auto s = m->create_component<Sender>("send");
    auto r = m->create_component<Receiver>("recv");
    m->create_connection<Passthrough>(
        "passt",
        s->get_name(),
        r->get_name());
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
}
