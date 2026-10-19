#pragma once
#include "components/AutoMoveD.hpp"
#include "components/GameGraphElements.hpp"
#include "machine/Actor.hpp"
#include "machine/MachineContext.hpp"
#include <raylib.h>
#include <string>

namespace components {

class Clock : public components::OComponent {
public:
    inline static const ::Color BODY_COLOR = ::YELLOW;
    inline static const ::Color DISPLAY_COLOR = ::BLACK;

public:
private:
    struct data {
        std::any m_msg_value;
        unsigned int count_to;
        bool set { };
        std::vector<std::string> recipents { };
        unsigned int counter { 0 };
        float font_size { };
    };
    AutoMoveD<data> m_d { };

public:
    Clock(
        std::string name,
        std::any&& msg_value,
        unsigned int count_to,
        int font_size);
    Clock(const Clock&) = delete;
    Clock& operator=(const Clock&) = delete;
    Clock(Clock&& o);
    Clock& operator=(Clock&& o);
    virtual ~Clock();

    virtual void draw() override;
    virtual void update() override;

    virtual machine::actor::Actor<void> poll(machine::Mctx) override;

    virtual const char*
    marshall_to_xml_name() const noexcept override;
    virtual void
    marshall_to_xml(pugi::xml_node&) const noexcept override;

private:
    static void setup(Clock& self);
};
}
