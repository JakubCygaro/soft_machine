#pragma once
#include "components/GameGraphElements.hpp"
#include "components/Memory.hpp"
#include "facelift/Fl.hpp"
#include "game/resources/Resources.hpp"

namespace components {

class ANNOTATE(fl::component) Display : public components::OComponent {
public:
    using display_msg_t = mem::MemoryWrite;

    inline static const ::Color BODY_COLOR = ::GRAY;
    inline static const ::Color SCREEN_COLOR = ::BLACK;

private:
    struct data {
        std::optional<int> val { std::nullopt };
        std::string val_rep { };
        ::Vector2 val_dims { };
        float font_size { };
    };
    AutoMoveD<data> m_d { };

public:
    inline Display(
        std::string name,
        int font_size)
        : components::OComponent(name)
        , m_d { { } }
    {
        m_d->font_size = font_size;
    }
    ANNOTATE(fl::use_ctor)
    inline Display(
        fl::string_param_t name,
        int font_size)
        : components::OComponent(std::string(name.data()))
        , m_d { { } }
    {
        m_d->font_size = font_size;
    }
    Display(const Display&) = delete;
    Display& operator=(const Display&) = delete;
    inline Display(Display&& o)
        : components::OComponent(std::move(o))
        , m_d { std::move(o.m_d) }
    {
    }
    inline Display& operator=(Display&& o)
    {
        components::OComponent::operator=(std::move(o));
        m_d = std::move(o.m_d);
        return *this;
    }
    inline virtual ~Display() { };

    virtual void draw() override;
    virtual void update() override;
    virtual machine::actor::Actor poll(machine::Mctx) override;

    virtual void set_size(const ::Vector2&);
    virtual const char*
    marshall_to_xml_name() const noexcept override;
    virtual void
    marshall_to_xml(pugi::xml_node&) const noexcept override;
};

}
