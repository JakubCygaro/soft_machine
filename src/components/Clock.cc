#include "components/Clock.hpp"
#include "common/reflect/Enum.hpp"
#include "game/resources/Resources.hpp"
#include <raylib.h>
#include <string>
namespace components {

void Clock::draw()
{
    using namespace game::resources;
    const auto count_to_sz = ::MeasureTextEx(
        get_node_font(),
        std::to_string(m_d->count_to).c_str(),
        m_d->font_size,
        default_font_spacing());
    const auto center = get_center();
    ::DrawRectangleRounded(
        this->m_bounds,
        0.2,
        10,
        BODY_COLOR);
    const ::Vector2 button_d = {
        .x = m_bounds.width * .8f,
        .y = m_bounds.height * .8f,
    };
    const ::Vector2 button_p = {
        .x = center.x - button_d.x / 2.0f,
        .y = center.y - button_d.y / 2.0f,
    };
}
void Clock::update()
{
}
void Clock::setup(Clock& self)
{
    using namespace game::resources;
    const auto count_to_sz = ::MeasureTextEx(
        get_node_font(),
        std::to_string(self.m_d->count_to).c_str(),
        self.m_d->font_size,
        default_font_spacing());
    self.m_bounds = ::Rectangle {
        .x = self.m_bounds.x,
        .y = self.m_bounds.y,
        .width = count_to_sz.x * 2.1f,
        .height = count_to_sz.y * 2.1f,
    };
}
std::any Clock::on_outcoming_connection(
    std::string_view conn_name,
    const machine::Connection*,
    std::any)
{
    this->m_d->recipents.push_back(std::string(conn_name));
    return nullptr;
}
machine::actor::Actor Clock::poll(machine::Mctx ctx)
{
    while (1) {
        co_await ctx.pause();
        if (m_d->counter == m_d->counter) {
            auto m = machine::message_t(m_d->m_msg_value);
            co_await ctx.broadcast(m_d->recipents, std::move(m));
            // for (const auto& r : m_d->recipents) {
            // }
        }
        m_d->counter = std::clamp(
            m_d->counter++,
            0u,
            m_d->count_to);
    }
}
const char*
Clock::marshall_to_xml_name() const noexcept
{
    return "clock";
}
void Clock::marshall_to_xml(pugi::xml_node& self) const noexcept
{
    OComponent::marshall_to_xml(self);
    self.append_attribute("count_to") = m_d->count_to;
    self.append_attribute("font_size") = m_d->font_size;
    auto value = self.append_child("value");
    if (auto ss = std::any_cast<std::string>(&this->m_d->m_msg_value); ss) {
        value.append_attribute("kind")
            .set_value(common::reflect::enum_to_string(MsgKind::String));
        value.text()
            .set(*ss);
    } else if (auto nn = std::any_cast<int>(&this->m_d->m_msg_value); nn) {
        value.append_attribute("kind")
            .set_value(common::reflect::enum_to_string(MsgKind::Number));
        value.text()
            .set(std::to_string(*nn));
    }
}
}
