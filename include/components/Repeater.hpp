#pragma once
#include "components/GameGraphElements.hpp"
#include "facelift/Fl.hpp"
#include <set>
namespace components {

// Takes a message from any incoming connection
// and repeats it on every outgoing connection
class ANNOTATE(fl::component) Repeater : public components::OComponent {
public:
    inline static const ::Color BODY_COLOR = ::GRAY;

private:
    struct data {
        std::set<std::string> from { };
        std::vector<std::string> to { };
        std::optional<unsigned int> max_in;
        std::optional<unsigned int> max_out;
    };
    AutoMoveD<data> m_d;

public:
    inline Repeater(
        std::string name,
        std::optional<unsigned int> max_in = std::nullopt,
        std::optional<unsigned int> max_out = std::nullopt)
        : components::OComponent(name)
        , m_d { { } }
    {
        m_d->max_in = max_in;
        m_d->max_out = max_out;
    }
    ANNOTATE(fl::use_ctor)
    inline Repeater(
        fl::string_param_t name)
        : components::OComponent(std::string(name.data()))
        , m_d { { } }
    {
    }
    Repeater(const Repeater&) = delete;
    Repeater& operator=(const Repeater&) = delete;
    inline Repeater(Repeater&& o)
        : components::OComponent(std::move(o))
        , m_d { std::move(o.m_d) }
    {
    }
    inline Repeater& operator=(Repeater&& o)
    {
        components::OComponent::operator=(std::move(o));
        m_d = std::move(o.m_d);
        return *this;
    }
    inline virtual ~Repeater() { };

    virtual void draw() override;
    virtual void update() override;
    virtual machine::actor::Actor poll(machine::Mctx) override;

    virtual void set_size(const ::Vector2&);

    virtual std::any on_incoming_connection(
        std::string_view,
        const machine::Connection*,
        std::any conn_msg) override;

    virtual std::any on_outcoming_connection(
        std::string_view,
        const machine::Connection*,
        std::any conn_msg) override;

    virtual const char*
    marshall_to_xml_name() const noexcept override;
    virtual void
    marshall_to_xml(pugi::xml_node&) const noexcept override;
};
}
