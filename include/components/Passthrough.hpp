#pragma once
#include "components/GameGraphElements.hpp"
#include "facelift/Fl.hpp"

namespace components {
class Passthrough : public components::OConnection {
private:
    ::Color m_color { ::BLACK };

public:
    Passthrough(
        std::string name,
        machine::Component* a,
        machine::Component* b,
        std::string in,
        std::string out);
    Passthrough(
        fl::string_param_t name,
        ANNOTATE(fl::ignore) machine::Component* a,
        ANNOTATE(fl::ignore) machine::Component* b,
        fl::string_param_t from,
        fl::string_param_t to);
    Passthrough(const Passthrough&) = delete;
    Passthrough& operator=(const Passthrough&) = delete;
    Passthrough(Passthrough&& o);
    Passthrough& operator=(Passthrough&& o);
    virtual ~Passthrough();
    virtual void draw() override;
    virtual machine::actor::Actor<void> poll(machine::Mctx ctx) override;

    virtual const char*
    marshall_to_xml_name() const noexcept override;
    // virtual void
    // marshall_to_xml(pugi::xml_node&) const noexcept override;
};
}
