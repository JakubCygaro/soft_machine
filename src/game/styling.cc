#include "game/Styling.hpp"
#include "common/Result.hpp"
#include "common/String.hpp"
#include "game/XmlMarshalling.hpp"
#include "game/resources/Resources.hpp"
#include <raylib.h>
#include <type_traits>
#include <variant>

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
font_node_t::parse_xml(pugi::xml_text txt)
{
    font_node_t n { };
    n.font_name = std::string(txt.as_string());
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
    auto font = node.child("font");
    if (auto unm = xml::unmarshall_node<xml_node_rep::font_node>(font); unm.iserr()) {
        return { unm.unwrap_err() };
    } else {
        xml.font = unm.unwrap();
    }
    if (auto sch = node.child("size"); sch) {
        auto unm = xml::unmarshall_node<xml_node_rep::size_node>(sch);
        if (unm.iserr())
            return { unm.unwrap_err() };
        auto sz = unm.unwrap();
        style.m_size = ::Vector2 {
            .x = sz.attrs->width,
            .y = sz.attrs->height
        };
    }
    if (auto apch = node.child("abs-pos"); apch) {
        auto unm = xml::unmarshall_node<xml_node_rep::abs_pos_node>(apch);
        if (unm.iserr())
            return { unm.unwrap_err() };
        auto apos = unm.unwrap();
        style.m_position = ::Vector2 {
            .x = apos.attrs->x,
            .y = apos.attrs->y
        };
    }
    // TODO: actual font loading but that comes much later
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
    , m_font_name { std::move(o.m_font_name) }
    , m_font { o.m_font }
    , m_fontd { o.m_fontd }
    , dependencies { std::move(o.dependencies) }
    , m_position { std::move(o.m_position) }
    , m_rel_pos { std::move(o.m_rel_pos) }
    , m_size { std::move(o.m_size) }
{
}
Style& Style::operator=(Style&& o)
{
    custom_parameters = std::move(o.custom_parameters);
    m_font = o.m_font;
    m_font_name = std::move(o.m_font_name);
    m_fontd = o.m_fontd;
    dependencies = std::move(o.dependencies);
    m_position = std::move(o.m_position);
    m_rel_pos = std::move(o.m_rel_pos);
    m_size = std::move(o.m_size);
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
const ::Vector2& Style::get_position() const
{
    return m_position;
}
const ::Vector2& Style::get_size() const
{
    return m_size;
}
std::optional<const StylingParameter&>
Style::get_styling_param(const std::string& name) const
{
    if (custom_parameters.contains(name)) {
        return { custom_parameters.at(name) };
    }
    return std::nullopt;
}
auto Style::style(Stylable* stylable) -> void
{
    stylable->set_pos(m_position);
    stylable->set_size(m_size);
}
auto Style::get_dependencies() -> const std::vector<std::string>&
{
    return dependencies;
}
const char*
Style::marshall_to_xml_name() const noexcept
{
    return "style";
}
void Style::marshall_to_xml(pugi::xml_node& self) const noexcept
{
    auto font = self.append_child("font");
    font.attribute("size") = m_fontd.size;
    font.attribute("spacing") = m_fontd.spacing;
    font.set_value(m_font_name);
    auto abs_pos = self.append_child("abs-pos");
    abs_pos.attribute("x") = m_position.x;
    abs_pos.attribute("y") = m_position.y;
    auto size = self.append_child("size");
    size.attribute("width") = m_position.x;
    size.attribute("height") = m_position.y;
    auto custom = self.append_child("custom");
    for (auto& [k, v] : custom_parameters) {
        auto param = custom.append_child("param");
        param.attribute("key") = k;
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::shared_ptr<AsStylingParameter>>) {
                param.attribute("value") = arg->to_value_string();
            } else {
                param.attribute("value") = arg;
            }
        },
            v);
    }
}
Stylable::~Stylable()
{
}
}
