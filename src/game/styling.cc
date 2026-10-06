#include "game/Styling.hpp"
#include "common/Result.hpp"
#include "common/String.hpp"
#include "game/XmlMarshalling.hpp"
#include "game/resources/Resources.hpp"
#include <raylib.h>

namespace game {
namespace {
    template <class T>
    std::optional<T> try_parse(const std::string_view trimmed)
    {
        T out;
        auto res = std::from_chars(
            trimmed.data(),
            trimmed.data() + trimmed.size(),
            out);
        if (res.ec == std::errc::invalid_argument) {
            return { };
        } else {
            return { out };
        }
    }
    StylingParameter styling_param_from_str(const std::string& str)
    {
        auto trimmed = common::trim(str);
        if (auto as_i = try_parse<int>(trimmed); as_i) {
            return *as_i;
        }
        if (auto as_f = try_parse<float>(trimmed); as_f) {
            return *as_f;
        }
        if (auto as_d = try_parse<double>(trimmed); as_d) {
            return *as_d;
        }
        return str;
    }
}
using font_node_t = Style::xml_node_rep::font_node;
Result<std::runtime_error, font_node_t>
font_node_t::parse_xml(const char* text)
{
    font_node_t n { };
    n.font_name = std::string(text);
    return { n };
}

Style::Style()
{
}
Result<std::runtime_error, Style>
Style::from_xml(pugi::xml_node& node)
{
    Style style { };
    xml_node_rep xml;
    if (auto unm = xml::unmarshall_node<xml_node_rep>(node); unm.iserr()) {
        return { unm.unwrap_err() };
    } else {
        xml = unm.unwrap();
    }
    style.m_font = ::GetFontDefault();
    style.m_fontd = FontData {
        .size = xml.font.attrs->size,
        .spacing = xml.font.attrs->spacing.value_or(
            game::resources::default_font_spacing()),
    };
    style.custom_parameters = std::move(xml.custom.params);
    return { std::move(style) };
}
auto Style::xml_node_rep::custom_node::unmarshall_self(const pugi::xml_node& n)
    -> Result<std::runtime_error, Unit>
{
    struct kvp_node {
        struct attrs {
            std::string key;
            std::string value;
        };
        xml::Attribute<attrs> attrs;
    };

    for (auto& child : n.children("param")) {
        kvp_node kvp;
        if (auto unm = xml::unmarshall_node<kvp_node>(child); unm.iserr()) {
            return { unm.unwrap_err() };
        } else {
            kvp = unm.unwrap();
        }
        this->params[kvp.attrs->key] = styling_param_from_str(kvp.attrs->value);
    }

    return { unit() };
}
Style::Style(Style&& o)
    : custom_parameters { std::move(o.custom_parameters) }
    , m_font { o.m_font }
    , m_fontd { o.m_fontd }
{
}
Style& Style::operator=(Style&& o)
{
    custom_parameters = std::move(o.custom_parameters);
    m_font = o.m_font;
    m_fontd = o.m_fontd;
    return *this;
}
Style::~Style()
{
}

const ::Font& Style::get_font() const
{
    return m_font;
}
const FontData& Style::get_font_data() const
{
    return m_fontd;
}
std::optional<const StylingParameter&>
Style::get_styling_param(const std::string& name) const
{
    if (custom_parameters.contains(name)) {
        return { custom_parameters.at(name) };
    }
    return std::nullopt;
}
}
