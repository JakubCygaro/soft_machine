#pragma once
#include "components/GameGraphElements.hpp"

namespace components {
class Passthrough : public components::OConnection {
private:
    ::Color m_color { ::BLACK };

public:
    inline Passthrough(machine::Component* a,
        machine::Component* b,
        std::string in,
        std::string out)
        : components::OConnection(a, b, in, out)
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
};
}
