#include "components/GameGraphElements.hpp"
#include "game/Scene.hpp"
#include "game/Xml.hpp"
#include "game/XmlMarshalling.hpp"
#include "game/resources/Resources.hpp"
#include "machine/Actor.hpp"
#include "machine/MachineGraph.hpp"
#include <any>
#include <fstream>
#include <iostream>
#include <memory>
#include <print>
#include <ranges>
#include <raylib.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>

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
            get_name().c_str(),
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
            this->get_name());
        m_color = ::LIME;
        std::flush(std::cout);
        auto [_, resp] = co_await ctx.recv();
        std::println("{} got response {}",
            this->get_name(),
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
            get_name().c_str(),
            get_pos(),
            10,
            5,
            ::BLACK);
    }
    virtual machine::actor::Actor
    poll(machine::Mctx ctx) override
    {
        std::println("{} entered", this->get_name());
        auto [_, m] = co_await ctx.recv();
        m_color = ::RED;
        auto s = std::any_cast<std::string>(m);
        std::println("{} got message {} ", this->get_name(), s);
        std::println("{} sending response", this->get_name());
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
    std::string xml;
    std::ifstream ifs("./test.xml");
    std::stringstream buf;
    buf << ifs.rdbuf();
    xml = buf.str();
    ::InitWindow(1000, 800, "Soft Machine");
    ::SetTargetFPS(60);
    game::resources::init_resources();
    auto res = game::populate_machine_from_xml(*m, xml);
    if (res.iserr()) {
        std::cout << std::move(res).unwrap_err().what() << std::endl;
        return 1;
    }
    // pugi::xml_document doc;
    // auto graph = doc.root().append_child("graph");
    // using namespace std::views;
    // using elem_t = machine::MachineGraph::comp_or_conn_ptr_t;
    // auto xmls = transform(m->get_elements(),
    //     [](const elem_t& e) {
    //         if (auto n = std::get_if<machine::Component*>(&e); n) {
    //             return dynamic_cast<game::xml::MarshallToXml*>(*n);
    //         } else {
    //             return dynamic_cast<
    //                 game::xml::MarshallToXml*>(
    //                 std::get<machine::Connection*>(e));
    //         }
    //     });
    // for (const auto& elem : xmls) {
    //     auto node = graph.append_child(elem->marshall_to_xml_name());
    //     elem->marshall_to_xml(node);
    // }
    // std::cout << "marshalled:\n";
    // doc.save(std::cout);
    // std::cout << std::endl;
    auto scene = game::GraphScene(std::move(m), { 0, 0, 1000, 800 });
    ::SetTraceLogLevel(LOG_ALL);
    while (!::WindowShouldClose()) {
        scene.update();
        ::BeginDrawing();
        scene.draw();
        ::EndDrawing();
    }
    game::resources::deinit_resources();
    ::CloseWindow();
}
