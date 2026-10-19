#include "components/Repeater.hpp"
#include "components/GameGraphElements.hpp"
#include <format>

namespace components {
Repeater::Repeater(
    std::string name,
    std::optional<unsigned int> max_in,
    std::optional<unsigned int> max_out)
    : components::OComponent(name)
    , m_d { { } }
{
    m_d->max_in = max_in;
    m_d->max_out = max_out;
}
Repeater::Repeater(
    fl::string_param_t name)
    : components::OComponent(std::string(name.data()))
    , m_d { { } }
{
}
Repeater::Repeater(Repeater&& o)
    : components::OComponent(std::move(o))
    , m_d { std::move(o.m_d) }
{
}
Repeater& Repeater::operator=(Repeater&& o)
{
    if (&o == this)
        return *this;
    components::OComponent::operator=(std::move(o));
    m_d = std::move(o.m_d);
    return *this;
}
Repeater::~Repeater() { }
void Repeater::draw()
{
    // const auto center = get_center();
    ::DrawRectangleRounded(
        this->m_bounds,
        0.2,
        10,
        BODY_COLOR);
}
void Repeater::update()
{
}
machine::actor::Actor<void> Repeater::poll(machine::Mctx ctx)
{
    while (1) {
        const auto [s, m] = co_await ctx.recv();
        if (!m_d->from.contains(s))
            continue;
        for (const auto& to : m_d->to) {
            auto cpy = std::any(m);
            co_await ctx.send(to, std::move(cpy));
        }
    }
}

void Repeater::set_size(const ::Vector2& sz)
{
    this->m_bounds.width = sz.x;
    this->m_bounds.height = sz.y;
}
std::any Repeater::on_incoming_connection(
    std::string_view n,
    const machine::Connection*,
    std::any)
{
    if (m_d->max_in && m_d->max_in < m_d->from.size() + 1) {
        throw std::runtime_error(
            std::format("repeater '{}': maximum in capacity exceeded"
                        " with connection '{}'!",
                get_name(),
                n));
    }
    m_d->from.insert(std::string(n));
    return nullptr;
}

std::any Repeater::on_outcoming_connection(
    std::string_view n,
    const machine::Connection*,
    std::any)
{
    if (m_d->max_out && m_d->max_out < m_d->to.size() + 1) {
        throw std::runtime_error(
            std::format("repeater '{}': maximum out capacity exceeded"
                        " with connection '{}'!",
                get_name(),
                n));
    }
    m_d->to.push_back(std::string(n));
    return nullptr;
}
const char*
Repeater::marshall_to_xml_name() const noexcept
{
    return "repeater";
}
void Repeater::marshall_to_xml(pugi::xml_node& self) const noexcept
{
    OComponent::marshall_to_xml(self);
    if (m_d->max_in) {
        self.append_attribute("max_in").set_value(*m_d->max_in);
    }
    if (m_d->max_out) {
        self.append_attribute("max_out").set_value(*m_d->max_out);
    }
    append_size_attributes(self, get_size());
}
}
