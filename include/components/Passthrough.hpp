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
    inline virtual ~Passthrough() { }
    virtual void draw() override;
    virtual machine::actor::Actor poll(machine::Mctx ctx) override;
};
}
