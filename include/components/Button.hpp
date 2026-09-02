#pragma once
#include "components/GameGraphElements.hpp"
#include "facelift/Fl.hpp"
#include "machine/Actor.hpp"
#include "machine/MachineContext.hpp"
#include <raylib.h>
#include <string>

namespace components {

class ANNOTATE(fl::component) Button : public components::OComponent {
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
    AutoMoveD<data> m_d { };

public:
    inline Button(
        std::string name,
        std::any&& msg_value)
        : components::OComponent(name)
        , m_d { { msg_value } }
    {
    }
    ANNOTATE(fl::use_ctor)
    inline Button(
        fl::string_param_t name,
        fl::string_param_t msg_value)
        : components::OComponent(std::string(name.data(), name.size()))
        , m_d { { nullptr } }
    {
        int out{};
        auto res = std::from_chars(
            msg_value.data(),
            msg_value.data() + msg_value.size(),
            out,
            10);
        if (res.ec == std::errc::invalid_argument) {
            m_d->m_msg_value = std::string(name.data(), name.size());
        } else {
            m_d->m_msg_value = out;
        }
    }
    Button(const Button&) = delete;
    Button& operator=(const Button&) = delete;
    inline Button(Button&& o)
        : components::OComponent(std::move(o))
        , m_d { std::move(o.m_d) }
    {
    }
    inline Button& operator=(Button&& o)
    {
        components::OComponent::operator=(std::move(o));
        m_d = std::move(o.m_d);
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

    virtual const char*
    marshall_to_xml_name() const noexcept override;
    virtual void
    marshall_to_xml(pugi::xml_node&) const noexcept override;
};
}
