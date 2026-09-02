#pragma once
#include "components/GameGraphElements.hpp"
#include "facelift/Fl.hpp"

namespace components {
class ANNOTATE(fl::component) Passthrough : public components::OConnection {
private:
    ::Color m_color { ::BLACK };

public:
    inline Passthrough(
        std::string name,
        machine::Component* a,
        machine::Component* b,
        std::string in,
        std::string out)
        : components::OConnection(name, a, b, in, out)
    {
    }
    ANNOTATE(fl::use_ctor)
    inline Passthrough(
        fl::string_param_t name,
        ANNOTATE(fl::ignore) machine::Component* a,
        ANNOTATE(fl::ignore) machine::Component* b,
        fl::string_param_t from,
        fl::string_param_t to)
        : components::OConnection(
              std::string(name.data()), a, b,
              std::string(from.data()), std::string(to.data()))
    {
    }
    inline Passthrough(const Passthrough&) = delete;
    inline Passthrough& operator=(const Passthrough&) = delete;
    inline Passthrough(Passthrough&& o)
        : components::OConnection(std::move(o))
        , m_color { o.m_color }
    {
    }
    inline Passthrough& operator=(Passthrough&& o)
    {
        components::OConnection::operator=(std::move(o));
        m_color = o.m_color;
        return *this;
    }
    inline virtual ~Passthrough() { }
    virtual void draw() override;
    virtual machine::actor::Actor poll(machine::Mctx ctx) override;

    virtual const char*
    marshall_to_xml_name() const noexcept override;
    // virtual void
    // marshall_to_xml(pugi::xml_node&) const noexcept override;
};
}
