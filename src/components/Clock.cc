#include "components/Clock.hpp"
#include "common/Ray.hpp"
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
    const auto counter_sz = ::MeasureTextEx(
        get_node_font(),
        std::to_string(m_d->counter).c_str(),
        m_d->font_size,
        default_font_spacing());
    ::DrawRectangleRec(
        this->m_bounds,
        BODY_COLOR);
    auto half = ::Rectangle {
        .x = m_bounds.x,
        .y = m_bounds.y,
        .width = m_bounds.width,
        .height = m_bounds.height / 2.0f,
    };
    auto half_center = common::ray::rec_center(half);
    ::DrawRectangleRounded(
        half,
        0.2,
        10,
        DISPLAY_COLOR);
    ::DrawTextEx(
        get_node_font(),
        std::to_string(m_d->count_to).c_str(),
        common::ray::text_centered_at(half_center, count_to_sz),
        default_node_font_size(),
        default_font_spacing(),
        ::WHITE);
    half.y += m_bounds.height - half.height;
    half_center = common::ray::rec_center(half);
    ::DrawRectangleRounded(
        half,
        0.2,
        10,
        DISPLAY_COLOR);
    ::DrawTextEx(
        get_node_font(),
        std::to_string(m_d->counter).c_str(),
        common::ray::text_centered_at(half_center, counter_sz),
        default_node_font_size(),
        default_font_spacing(),
        ::WHITE);
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
machine::actor::Actor Clock::poll(machine::Mctx ctx)
{
    while (1) {
        co_await ctx.pause();
        if (++m_d->counter > m_d->count_to)
            m_d->counter = 0;
        if (m_d->counter == m_d->count_to) {
            auto m = machine::message_t(m_d->m_msg_value);
            co_await ctx.broadcast(std::move(m));
        }
        std::flush(std::cout);
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
            .set_value(common::reflect::enum_to_string(ValueMsgKind::String));
        value.text()
            .set(*ss);
    } else if (auto nn = std::any_cast<int>(&this->m_d->m_msg_value); nn) {
        value.append_attribute("kind")
            .set_value(common::reflect::enum_to_string(ValueMsgKind::Number));
        value.text()
            .set(std::to_string(*nn));
    }
}
}
