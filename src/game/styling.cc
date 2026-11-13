#include "game/Styling.hpp"
#include "common/Optional.hpp"
#include "common/Ray.hpp"
#include "common/Result.hpp"
#include "common/String.hpp"
#include "common/reflect/Enum.hpp"
#include "game/Editable.hpp"
#include "game/StylingContext.hpp"
#include "game/XmlMarshalling.hpp"
#include "game/resources/Resources.hpp"
#include "imgui.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <functional>
#include <optional>
#include <raylib.h>
#include <raymath.h>
#include <sstream>
#include <string>
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
namespace {
    std::string trim_lisp_text(const pugi::xml_text& txt)
    {
        std::string ret;
        ret.reserve(std::strlen(txt.get()));
        std::stringstream ss;
        ss << txt.get();
        for (std::string line; std::getline(ss, line);) {
            ret += common::trim(line);
            ret += '\n';
        }
        return ret;
    }
}
Result<std::runtime_error, Style>
Style::from_xml(pugi::xml_node& node)
{
    Style style { };
    xml_node_rep xml;
    auto font = node.child("font");
    if (!font) {
        xml.font.font_name = "default";
        xml.font.attrs->size = game::resources::default_node_font_size();
        xml.font.attrs->spacing = game::resources::default_font_spacing();
    } else if (auto unm = xml::unmarshall_node<xml_node_rep::font_node>(
                   font);
        unm.iserr()) {
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
    style.m_override_abs_pos = false;
    if (auto apch = node.child("abs-pos"); apch) {
        auto unm = xml::unmarshall_node<xml_node_rep::abs_pos_node>(apch);
        if (unm.iserr())
            return { unm.unwrap_err() };
        auto apos = unm.unwrap();
        style.m_position = ::Vector2 {
            .x = apos.attrs->x,
            .y = apos.attrs->y
        };
        style.m_override_abs_pos |= true;
    }
    if (auto rpch = node.child("rel-pos"); rpch) {
        auto unm = xml::unmarshall_node<xml_node_rep::rel_pos_node>(rpch);
        if (unm.iserr())
            return { unm.unwrap_err() };
        auto rpos = unm.unwrap();
        style.m_rel_pos = RelativePos {
            .rel_to = rpos.attrs->rel,
            .side = rpos.attrs->side,
            .distance = rpos.attrs->distance
        };
        style.m_override_abs_pos |= true;
    }
    if (auto lispch = node.child("lisp"); lispch) {
        style.m_lisp = trim_lisp_text(lispch.text());
        // style.m_lisp = lispch.text().get();
        // style.m_lisp = common::trim(*style.m_lisp);
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
    , m_position { std::move(o.m_position) }
    , m_rel_pos { std::move(o.m_rel_pos) }
    , m_size { std::move(o.m_size) }
    , m_lisp { std::move(o.m_lisp) }
    , m_override_abs_pos { o.m_override_abs_pos }
{
}
Style& Style::operator=(Style&& o)
{
    custom_parameters = std::move(o.custom_parameters);
    m_font = o.m_font;
    m_font_name = std::move(o.m_font_name);
    m_fontd = o.m_fontd;
    m_position = std::move(o.m_position);
    m_rel_pos = std::move(o.m_rel_pos);
    m_size = std::move(o.m_size);
    m_lisp = std::move(o.m_lisp);
    m_override_abs_pos = o.m_override_abs_pos;
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
std::optional<const ::Vector2&> Style::get_position() const
{
    return m_position;
}
std::optional<const ::Vector2&> Style::get_size() const
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
auto Style::style(StylingContext* sctx, Stylable* stylable) -> void
{
    m_sctx = sctx;
    m_styllable = stylable;
    if (m_lisp) {
        Style::set_current_instance(&(*this));
        sctx->get_style_scheme().execute_string(*m_lisp);
        Style::clear_current_instance();
    }
    if (m_size)
        stylable->set_size(*m_size);
    if (m_rel_pos) {
        if (auto d = sctx->query_component(m_rel_pos->rel_to, false); d) {
            const auto& dist = m_rel_pos->distance;
            const auto b = stylable->get_bounds();
            using enum game::RelativePos::Side;
            using enum components::AttachPt;
            ::Rectangle pos;
            switch (m_rel_pos->side) {
            case Top:
                pos = common::ray::rec_centered_at(
                    b,
                    d->get_att_point(TC));
                pos.y -= b.height / 2.0
                    + dist;
                break;
            case Bottom:
                pos = common::ray::rec_centered_at(
                    b,
                    d->get_att_point(BC));
                pos.y += b.height / 2.0
                    + dist;
                break;
            case Right:
                pos = common::ray::rec_centered_at(
                    b,
                    d->get_att_point(R));
                pos.x += b.width / 2.0
                    + dist;
                break;
            case Left:
                pos = common::ray::rec_centered_at(
                    b,
                    d->get_att_point(L));
                pos.x -= b.width / 2.0
                    + dist;
                break;
            }
            stylable->set_pos({
                pos.x + m_position.value_or(::Vector2Zero()).x,
                pos.y + m_position.value_or(::Vector2Zero()).y,
            });
        } else {
            ::TraceLog(::LOG_ERROR, "[Styling] Unable to find styling reference '%s'",
                m_rel_pos->rel_to.c_str());
        }
    } else if (m_position) {
        if (!m_override_abs_pos && *m_position != stylable->get_pos())
            m_position = stylable->get_pos();
        stylable->set_pos(*m_position);
    }
    m_sctx = nullptr;
    m_styllable = nullptr;
}
auto Style::should_rebuild_dep_graph() -> bool
{
    const auto ret = m_should_rebuild_dep;
    this->m_should_rebuild_dep = false;
    return ret;
}
const char*
Style::marshall_to_xml_name() const noexcept
{
    return "style";
}
void Style::marshall_to_xml(pugi::xml_node& self) const noexcept
{
    auto font = self.append_child("font");
    font.append_attribute("size").set_value(m_fontd.size);
    font.append_attribute("spacing").set_value(m_fontd.spacing);
    font.set_value(m_font_name);
    if (m_position) {
        auto abs_pos = self.append_child("abs-pos");
        abs_pos.append_attribute("x").set_value(m_position->x);
        abs_pos.append_attribute("y").set_value(m_position->y);
    }
    if (m_size) {
        auto size = self.append_child("size");
        size.append_attribute("width").set_value(m_size->x);
        size.append_attribute("height").set_value(m_size->y);
    }
    auto custom = self.append_child("custom");
    for (auto& [k, v] : custom_parameters) {
        auto param = custom.append_child("param");
        param.append_attribute("key").set_value(k);
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::shared_ptr<AsStylingParameter>>) {
                param.append_attribute("value").set_value(arg->to_value_string());
            } else {
                param.append_attribute("value").set_value(arg);
            }
        },
            v);
    }
    if (m_lisp) {
        auto lisp = self.append_child("lisp");
        lisp.text() = *m_lisp;
    }
}
Stylable::~Stylable()
{
}
namespace {
    inline void log_instance_not_set_err()
    {
        ::TraceLog(::LOG_ERROR, "[STYLING-LISP] Style instance not set!");
    }
}
void Style::set_current_instance(Style* s)
{
    s_instance = s;
}
void Style::clear_current_instance()
{
    s_instance = nullptr;
}

inline auto with_instance_or_err(Style* inst_ptr, auto func)
{
    if (inst_ptr)
        return func(inst_ptr);
    else {
        log_instance_not_set_err();
        if constexpr (std::is_void<decltype(func(inst_ptr))>::value) {
            return;
        } else {
            return decltype(func(inst_ptr)) { };
        }
    }
}

void Style::instance_set_abs_pos(::Vector2 pos)
{
    with_instance_or_err(s_instance, [=](Style* s) {
        s->m_position = pos;
    });
}
::Vector2 Style::instance_get_abs_pos(std::optional<const std::string&> other)
{
    if (other) {
        return with_instance_or_err(s_instance, [&](auto* s) {
            if (auto c = s->m_sctx->query_component(*other); c) {
                return c->get_pos();
            }
            return ::Vector2Zero();
        });
    }
    return with_instance_or_err(s_instance, [&](auto* s) {
        return s->m_position.value_or(::Vector2Zero());
    });
}
void Style::instance_set_rel_pos(RelativePos rp)
{
    return with_instance_or_err(s_instance, [&](auto* s) {
        s->m_rel_pos = rp;
    });
}
std::optional<const RelativePos&> Style::instance_get_rel_pos()
{
    if (s_instance)
        return s_instance->m_rel_pos;
    log_instance_not_set_err();
    return { };
}
bool Style::instance_exists(const std::string& n)
{
    return with_instance_or_err(s_instance, [&](Style* s) {
        return s->m_sctx->exists(n);
    });
}
bool Style::instance_is_component(std::optional<const std::string&> n)
{
    return with_instance_or_err(s_instance, [&](Style* s) {
        if (n) {
            return s->m_sctx->is_component(*n);
        } else {
            return !!dynamic_cast<components::OComponent*>(s->m_styllable);
        }
    });
}
bool Style::instance_is_connection(std::optional<const std::string&> n)
{
    return with_instance_or_err(s_instance, [&](Style* s) {
        if (n) {
            return s->m_sctx->is_connector(*n);
        } else {
            return !!dynamic_cast<components::OConnection*>(s->m_styllable);
        }
    });
}
void Style::instance_set_font_size(float sz)
{
    with_instance_or_err(s_instance, [=](Style* s) {
        s->m_fontd.size = sz;
    });
}
float Style::instance_get_font_size(std::optional<const std::string&> n)
{
    return with_instance_or_err(s_instance, [=](Style* s) {
        if (n) {
            return s->m_sctx->query_style(*n)
                .and_then([](const auto& st) { return std::optional {
                                                   st.get_font_data().size
                                               }; })
                .value_or(0.0f);
        } else {
            return s->m_fontd.size;
        }
    });
}
void Style::instance_set_font(const std::string&)
{
    // TODO: fonts
}
const std::string& Style::instance_get_font()
{
    static std::string _dummy = "defult";
    return _dummy;
}
void Style::instance_set_param(const std::string& name, StylingParameter param)
{
}
std::optional<const StylingParameter&>
Style::instance_get_param(const std::string& name)
{
}
std::unique_ptr<Editor> Style::get_editor_object()
{
    std::array<float, 2> _abs_pos = { };
    std::array<float, 2> _size = { };
    std::array<char, 256> _rel_pos_r = { };
    std::array<char, 256> _rel_pos_s = { };
    // TODO: this probably needs to be dynamically resizable
    std::array<char, 512> _lisp { };
    common::visit_opt(m_position, [&](const auto& p) {
        _abs_pos[0] = p.x;
        _abs_pos[1] = p.y;
    });
    common::visit_opt(m_size, [&](const auto& sz) {
        _size[0] = sz.x;
        _size[1] = sz.y;
    });
    common::visit_opt(m_lisp, [&](const auto& lisp) {
        _lisp.fill(0);
        std::copy(
            lisp.begin(),
            lisp.end(),
            _lisp.begin());
    });
    float _rel_pos_d;
    common::visit_opt(m_rel_pos, [&](const RelativePos& rp) {
        _rel_pos_d = rp.distance;
        std::strcpy(_rel_pos_r.data(), rp.rel_to.c_str());
        std::strcpy(_rel_pos_s.data(), common::reflect::enum_to_string(rp.side).c_str());
    });
    bool lisp_changed = false;
    float _font_size = m_fontd.size;
    return LambdaEditor::make([=, this] mutable -> bool {
        bool flag = false;
        if (this->m_override_abs_pos && m_position)
            _abs_pos[0] = m_position->x, _abs_pos[1] = m_position->y;
        if (m_size)
            _size[0] = m_size->x, _size[1] = m_size->y;
        if (ImGui::CollapsingHeader("styling")) {
            ImGui::SeparatorText("Font size");
            if ((flag |= ImGui::InputFloat("Font Size", &_font_size))) {
                this->m_fontd.size = _font_size;
            }
            ImGui::SeparatorText("Size");
            if ((flag |= ImGui::InputFloat2("Size", _size.data()))) {
                this->m_size = ::Vector2 {
                    _size[0],
                    _size[1],
                };
            }
            ImGui::SeparatorText("Absolute position");
            if ((flag |= ImGui::InputFloat2("Position", _abs_pos.data()))) {
                this->m_position = ::Vector2 {
                    _abs_pos[0],
                    _abs_pos[1],
                };
            }
            ImGui::SameLine();
            if (ImGui::Checkbox("Style override", &this->m_override_abs_pos)) {
                flag |= true;
                if (this->m_override_abs_pos)
                    this->m_position = ::Vector2 {
                        _abs_pos[0],
                        _abs_pos[1],
                    };
            }
            ImGui::SeparatorText("Relative position");
            ImGui::InputText("Depends on", _rel_pos_r.data(), _rel_pos_r.size());
            if (this->m_rel_pos
                && ImGui::InputText("Side", _rel_pos_s.data(), _rel_pos_s.size())) {
            }
            if (this->m_rel_pos
                && ImGui::InputFloat("Distance", &_rel_pos_d)) {
            }
            if ((std::strlen(_rel_pos_r.data()) != 0 || this->m_rel_pos)
                && ImGui::Button("Change")) {
                using enum common::reflect::MatchCase;
                using namespace common;
                RelativePos rp {
                    .rel_to = std::string(
                        _rel_pos_r.data(), std::strlen(_rel_pos_r.data())),
                    .side = reflect::string_to_enum<
                        game::RelativePos::Side, IgnoreCase>(_rel_pos_s.data())
                        .value_or(game::RelativePos::Side::Left),
                    .distance = _rel_pos_d,
                };
                this->m_rel_pos = std::move(rp);
                const auto as_str = reflect::enum_to_string(rp.side);
                _rel_pos_s.fill('\0');
                std::copy(
                    as_str.begin(),
                    as_str.end(),
                    _rel_pos_s.begin());
                // _rel_pos_s = reflect::enum_to_string(rp.side);
                flag |= true;
            }
            ImGui::SeparatorText("LISP source");
            if (ImGui::InputTextMultiline("LISP", _lisp.data(), _lisp.size())) {
                lisp_changed |= true;
            }
            if (lisp_changed && ImGui::Button("Save LISP")) {
                this->m_lisp = std::string(_lisp.data());
                this->m_should_rebuild_dep = true;
                lisp_changed = false;
                flag |= true;
            }
        }
        return flag;
    });
}
}
