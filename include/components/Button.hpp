#pragma once
#include "components/AutoMoveD.hpp"
#include "components/GameGraphElements.hpp"
#include "facelift/Fl.hpp"
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
private:
    struct data {
        std::any m_msg_value;
        bool set { };
        std::vector<std::string> recipents { };
    };
    AutoMoveD<data> m_d { };

public:
    Button(
        std::string name,
        std::any&& msg_value);
    Button(
        fl::string_param_t name,
        fl::string_param_t msg_value);
    Button(const Button&) = delete;
    Button& operator=(const Button&) = delete;
    Button(Button&& o);
    Button& operator=(Button&& o);
    virtual ~Button();

    virtual void draw() override;
    virtual void update() override;
    virtual void on_input() override;

    virtual std::any on_outcoming_connection(
        std::string_view,
        const machine::Connection*,
        std::any) override;
    virtual machine::actor::Actor<void> poll(machine::Mctx) override;

    virtual void set_size(const ::Vector2&) override;

    virtual const char*
    marshall_to_xml_name() const noexcept override;
    virtual void
    marshall_to_xml(pugi::xml_node&) const noexcept override;
};
}
