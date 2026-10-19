#pragma once
#include "common/Result.hpp"
#include "common/reflect/Enum.hpp"
#include "game/Drawable.hpp"
// #include "game/Xml.hpp"
#include "game/XmlMarshalling.hpp"
#include "machine/Component.hpp"
#include "machine/Connection.hpp"
#include <pugixml.hpp>
#include <raylib.h>
#include <raymath.h>
#include <string_view>

namespace components {

enum class AttachPt {
    TL,
    TC,
    TR,
    L,
    C,
    R,
    BL,
    BC,
    BR
};

enum class ValueMsgKind {
    String,
    Number,
};

Result<std::runtime_error, std::any>
parse_value_msg_kind(const ValueMsgKind& kind, const std::string& text);

void append_position_node(pugi::xml_node& to, const ::Vector2& pos);
void append_size_attributes(pugi::xml_node& to, const ::Vector2& sz);

class OComponent : public machine::Component,
                   public game::Object,
                   public game::xml::MarshallToXml {
public:
    const ::Rectangle DEFAULT_BOUNDS = ::Rectangle {
        .x = 0,
        .y = 0,
        .width = 100,
        .height = 100
    };

protected:
    ::Rectangle m_bounds { };

public:
    OComponent(std::string name);
    OComponent(const OComponent&) = delete;
    OComponent& operator=(const OComponent&) = delete;
    OComponent(OComponent&& o);
    OComponent& operator=(OComponent&& o);
    virtual ~OComponent();

    ::Vector2 get_pos() const;
    virtual void set_pos(const ::Vector2& p) override;
    virtual void set_size(const ::Vector2& s) override;
    ::Vector2 get_size() const;
    ::Rectangle get_bounds() const;
    ::Vector2 get_center() const;
    virtual bool check_point_collision(const ::Vector2& point) const override;
    virtual ::Vector2 get_att_point(AttachPt att) const;
    virtual const char* marshall_to_xml_name() const noexcept override;
    virtual void marshall_to_xml(pugi::xml_node& self) const noexcept override;
    virtual bool draw_edit_window() override;
};
class OConnection : public machine::Connection,
                    public game::Object,
                    public game::xml::MarshallToXml {
public:
    enum class Attachment {
        End,
        Start
    };

protected:
    ::Vector2 m_start_pos { },
        m_end_pos { };
    AttachPt m_start_ap { }, m_end_ap { };

public:
    OConnection(
        std::string name,
        machine::Component* start,
        machine::Component* end,
        std::string in,
        std::string out);
    OConnection(const OConnection&) = delete;
    OConnection& operator=(const OConnection&) = delete;
    OConnection(OConnection&& o);
    OConnection& operator=(OConnection&& o);
    virtual ~OConnection();

private:
    static void _set_from_attp(OConnection& self, const AttachPt& ap);
    static void _set_to_attp(OConnection& self, const AttachPt& ap);

public:
    virtual void set_from_attp(const AttachPt& ap);
    virtual void set_to_attp(const AttachPt& ap);
    virtual AttachPt get_from_attp();
    virtual AttachPt get_to_attp();
    virtual ::Vector2 get_start_pos();
    virtual ::Vector2 get_start_pos() const;
    virtual ::Vector2 get_end_pos();
    virtual ::Vector2 get_end_pos() const;

    // called by the graph when either the from or the end component
    // has changed in a relevant way
    virtual void on_component_changed(const Attachment& a);
    virtual bool check_point_collision(const ::Vector2& point) const override;
    virtual const char*
    marshall_to_xml_name() const noexcept override;
    virtual void
    marshall_to_xml(pugi::xml_node& self) const noexcept override;
    virtual bool draw_edit_window() override;
    virtual void on_notified() override;
};
}
