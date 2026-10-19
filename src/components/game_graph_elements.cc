#include "common/String.hpp"
#include "components/GameGraphElements.hpp"
#include "imgui.h"
namespace components {
void append_position_node(pugi::xml_node& to, const ::Vector2& pos)
{
    auto p = to.append_child("position");
    p.append_attribute("x").set_value(pos.x);
    p.append_attribute("y").set_value(pos.y);
}
void append_size_attributes(pugi::xml_node& to, const ::Vector2& sz)
{
    to.append_attribute("width").set_value(sz.x);
    to.append_attribute("height").set_value(sz.y);
}

Result<std::runtime_error, std::any>
parse_value_msg_kind(const ValueMsgKind& kind, const std::string& text)
{
    using enum components::ValueMsgKind;
    std::any msg;
    switch (kind) {
    case String: {
        msg = common::trim(text);
    } break;
    case Number: {
        auto trimmed = common::trim(text);
        int out;
        auto res = std::from_chars(
            trimmed.data(),
            trimmed.data() + trimmed.size(),
            out,
            10);
        if (res.ec == std::errc::invalid_argument) {
            return { std::runtime_error(
                std::format("Failed to parse clock value node as a number")) };
        }
        msg = out;
    } break;
    }
    return { msg };
}

OComponent::OComponent(std::string name)
    : machine::Component(name)
    , m_bounds { DEFAULT_BOUNDS }
{
}
OComponent::OComponent(OComponent&& o)
    : machine::Component(std::move(o))
    , m_bounds { o.m_bounds }
{
}
OComponent& OComponent::operator=(OComponent&& o)
{
    if (&o == this)
        return *this;
    machine::Component::operator=(std::move(o));
    m_bounds = o.m_bounds;
    return *this;
}
OComponent::~OComponent() { }

::Vector2 OComponent::get_pos() const
{
    return ::Vector2 { .x = m_bounds.x, .y = m_bounds.y };
}
void OComponent::set_pos(const ::Vector2& p)
{
    this->m_bounds.x = p.x;
    this->m_bounds.y = p.y;
}
void OComponent::set_size(const ::Vector2& s)
{
    m_bounds.width = s.x;
    m_bounds.height = s.y;
};
::Vector2 OComponent::get_size() const
{
    return ::Vector2 { .x = m_bounds.width, .y = m_bounds.height };
}
::Rectangle OComponent::get_bounds() const
{
    return m_bounds;
}
::Vector2 OComponent::get_center() const
{
    return { m_bounds.x + m_bounds.width / 2, m_bounds.y + m_bounds.height / 2 };
}
bool OComponent::check_point_collision(const ::Vector2& point) const
{
    return ::CheckCollisionPointRec(point, m_bounds);
}
::Vector2 OComponent::get_att_point(AttachPt att) const
{
    ::Vector2 p = get_center();
    switch (att) {
    case AttachPt::TL: {
        p = get_pos();
    } break;
    case AttachPt::TC: {
        p = get_pos();
        p.x += get_size().x / 2;
    } break;
    case AttachPt::TR: {
        p = get_pos();
        p.x += get_size().x;
    } break;
    case AttachPt::R: {
        p = get_pos();
        p.x += get_size().x;
        p.y += get_size().y / 2;
    } break;
    case AttachPt::L: {
        p = get_pos();
        p.y += get_size().y / 2;
    } break;
    case AttachPt::BL: {
        p = get_pos();
        p.y += get_size().y;
    } break;
    case AttachPt::BC: {
        p = get_pos();
        p.x += get_size().x / 2;
        p.y += get_size().y;
    } break;
    case AttachPt::BR: {
        p = get_pos();
        p.x += get_size().x;
        p.y += get_size().y;
    } break;
    default:
        break;
    };
    return p;
}
const char*
OComponent::marshall_to_xml_name() const noexcept
{
    return "";
}
void OComponent::marshall_to_xml(pugi::xml_node& self) const noexcept
{
    append_position_node(self, get_pos());
    self.append_attribute("name").set_value(get_name());
    if (auto s = get_style(); s) {
        auto style = self.append_child(s->marshall_to_xml_name());
        s->marshall_to_xml(style);
    }
}
bool OComponent::draw_edit_window()
{
    auto notify = false;
    static float pos[2] = { m_bounds.x, m_bounds.y };
    if ((notify |= ImGui::InputFloat2("Position", pos))) {
        m_bounds.x = pos[0];
        m_bounds.y = pos[1];
    }
    return notify;
}

OConnection::OConnection(
    std::string name,
    machine::Component* start,
    machine::Component* end,
    std::string in,
    std::string out)
    : machine::Connection(name, start, end, in, out)
{
    _set_from_attp(*this, AttachPt::C);
    _set_to_attp(*this, AttachPt::C);
}
OConnection::OConnection(OConnection&& o)
    : machine::Connection(std::move(o))
    , m_start_pos { o.m_start_pos }
    , m_end_pos { o.m_end_pos }
    , m_start_ap { o.m_start_ap }
    , m_end_ap { o.m_end_ap }
{
}
OConnection& OConnection::operator=(OConnection&& o)
{
    if (&o == this)
        return *this;
    machine::Connection::operator=(std::move(o));
    m_start_pos = o.m_start_pos;
    m_end_pos = o.m_end_pos;
    m_start_ap = o.m_start_ap;
    m_end_ap = o.m_end_ap;
    return *this;
}
OConnection::~OConnection() { }

void OConnection::_set_from_attp(OConnection& self, const AttachPt& ap)
{
    self.m_start_ap = ap;
    if (auto s = dynamic_cast<const OComponent*>(self.get_start())) {
        self.m_start_pos = s->get_att_point(ap);
    }
}
void OConnection::_set_to_attp(OConnection& self, const AttachPt& ap)
{
    self.m_end_ap = ap;
    if (auto e = dynamic_cast<const OComponent*>(self.get_end())) {
        self.m_end_pos = e->get_att_point(ap);
    }
}

void OConnection::set_from_attp(const AttachPt& ap)
{
    _set_from_attp(*this, ap);
};
void OConnection::set_to_attp(const AttachPt& ap)
{
    _set_to_attp(*this, ap);
};
AttachPt OConnection::get_from_attp()
{
    return m_start_ap;
};
AttachPt OConnection::get_to_attp()
{
    return m_end_ap;
};
::Vector2 OConnection::get_start_pos()
{
    return m_start_pos;
};
::Vector2 OConnection::get_start_pos() const
{
    return m_start_pos;
};
::Vector2 OConnection::get_end_pos()
{
    return m_end_pos;
}
::Vector2 OConnection::get_end_pos() const
{
    return m_end_pos;
}
// called by the graph when either the from or the end component
// has changed in a relevant way
void OConnection::on_component_changed(const Attachment& a)
{
    switch (a) {
    case Attachment::Start:
        set_from_attp(this->m_start_ap);
        break;
    case Attachment::End:
        set_to_attp(this->m_end_ap);
        break;
    }
}
bool OConnection::check_point_collision(const ::Vector2& point) const
{
    return ::CheckCollisionPointLine(point, m_start_pos, m_end_pos, 5);
}
const char*
OConnection::marshall_to_xml_name() const noexcept
{
    return "";
}
void OConnection::marshall_to_xml(pugi::xml_node& self) const noexcept
{
    self.append_attribute("name").set_value(get_name());
    auto from = self.append_child("from");
    from.append_attribute("name")
        .set_value(out);
    from.append_attribute("at")
        .set_value(common::reflect::enum_to_string(m_start_ap));

    auto to = self.append_child("to");
    to.append_attribute("name")
        .set_value(out);
    to.append_attribute("at")
        .set_value(common::reflect::enum_to_string(m_end_ap));
    if (auto s = get_style(); s) {
        auto style = self.append_child(s->marshall_to_xml_name());
        s->marshall_to_xml(style);
    }
}
bool OConnection::draw_edit_window()
{
    return false;
}
void OConnection::on_notified()
{
    on_component_changed(Attachment::Start);
    on_component_changed(Attachment::End);
};
}
