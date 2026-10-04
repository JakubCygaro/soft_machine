#pragma once
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
    inline Clock(
        std::string name,
        std::any&& msg_value,
        unsigned int count_to,
        int font_size)
        : components::OComponent(name)
        , m_d { { msg_value, count_to } }
    {
        this->m_d->font_size = font_size;
    }
    Clock(const Clock&) = delete;
    Clock& operator=(const Clock&) = delete;
    inline Clock(Clock&& o)
        : components::OComponent(std::move(o))
        , m_d { std::move(o.m_d) }
    {
    }
    inline Clock& operator=(Clock&& o)
    {
        components::OComponent::operator=(std::move(o));
        m_d = std::move(o.m_d);
        return *this;
    }
    inline virtual ~Clock() { };

    virtual void draw() override;
    virtual void update() override;
    static void setup(Clock& self);

    virtual machine::actor::Actor poll(machine::Mctx) override;

    virtual const char*
    marshall_to_xml_name() const noexcept override;
    virtual void
    marshall_to_xml(pugi::xml_node&) const noexcept override;
};
}
