#pragma once
#include "components/GameGraphElements.hpp"
#include "machine/Actor.hpp"
#include "machine/MachineContext.hpp"
#include <raylib.h>
#include <string>

namespace components {

class Button : public components::OComponent {
public:
    inline static const ::Color BODY_COLOR = ::GRAY;
    inline static const ::Color BUTTON_INACTIVE_COLOR = { 189, 13, 13, 255 };
    inline static const ::Color BUTTON_ACTIVE_COLOR = { 255, 0, 0, 255 };

public:
    enum class MsgKind {
        String,
        Number,
    };

private:
    struct data {
        std::any m_msg_value;
        bool set { };
        std::vector<std::string> recipents { };
    };
    AutoMoveD<data> d { };

public:
    inline Button(
        std::string name,
        std::any&& msg_value)
        : components::OComponent(name)
        , d { { msg_value } }
    {
    }
    Button(const Button&) = delete;
    Button& operator=(const Button&) = delete;
    inline Button(Button&& o)
        : components::OComponent(std::move(o))
        , d { std::move(o.d) }
    {
    }
    inline Button& operator=(Button&& o)
    {
        components::OComponent::operator=(std::move(o));
        d = std::move(o.d);
        return *this;
    }
    inline virtual ~Button() { };

    virtual void draw() override;
    virtual void update() override;
    virtual std::any on_outcoming_connection(
        std::string_view,
        const machine::Connection*,
        std::any) override;
    virtual machine::actor::Actor poll(machine::Mctx) override;

    virtual void set_size(const ::Vector2&);
};
}
