#include "game/Styling.hpp"

namespace game {
StyleBuilder::StyleBuilder()
    : m_style { Style() }
{
}
StyleBuilder::StyleBuilder(StyleBuilder&& o)
    : m_style { std::move(o.m_style) }
{
}
StyleBuilder& StyleBuilder::operator=(StyleBuilder&& o)
{
    m_style = std::move(o.m_style);
    return *this;
}
StyleBuilder::~StyleBuilder()
{
}
StyleBuilder& StyleBuilder::with_font(::Font f)
{
    m_style.m_font = f;
    return *this;
}
StyleBuilder& StyleBuilder::with_font_data(FontData fd)
{
    m_style.m_fontd = fd;
    return *this;
}
StyleBuilder& StyleBuilder::with_custom_param(
    const std::string& name,
    StylingParameter&& param)
{
    m_style.custom_parameters[name] = std::move(param);
    return *this;
}
Style StyleBuilder::build_style() &&
{
    return std::move(m_style);
}
}
