#include "components/GameGraphElements.hpp"
#include "machine/Actor.hpp"
#include "machine/MachineGraph.hpp"
#include "game/Scene.hpp"
#include "game/Xml.hpp"
#include <any>
#include <fstream>
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

class Sender : public components::OComponent {
    ::Color m_color { ::LIME };

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
        ::DrawRectangleRec(m_bounds, m_color);
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
        std::flush(std::cout);
        m_color = ::GREEN;
        co_await ctx.send("passt",
            std::string("Siema eniu"));
        std::println("{} came back from pause, now will wait for response",
            this->name);
        m_color = ::LIME;
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
    ::Color m_color { ::ORANGE };

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
        ::DrawRectangleRec(m_bounds, m_color);
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
        auto [_, m] = co_await ctx.recv();
        m_color = ::RED;
        auto s = std::any_cast<std::string>(m);
        std::println("{} got message {} ", this->name, s);
        std::println("{} sending response", this->name);
        co_await ctx.send("passt", std::string("Eniu siema"));
        m_color = ::ORANGE;
        std::flush(std::cout);
        while (true)
            co_await ctx.pause();
    }
};
int main(void)
{
    auto m = std::unique_ptr<machine::MachineGraph>(
        machine::MachineGraph::create());
    // auto s = m->create_component<Sender>("send");
    // auto r = m->create_component<Receiver>("recv");
    // m->create_connection<components::Passthrough>(
    //     "passt",
    //     s->get_name(),
    //     r->get_name());
    std::string xml;
    std::ifstream("./tsm.xml") >> xml;
    game::populate_machine_from_xml(*m, xml);
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
