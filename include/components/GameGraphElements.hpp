#pragma once
#include "common/reflect/Enum.hpp"
#include "game/Drawable.hpp"
#include "game/Xml.hpp"
#include "game/XmlMarshalling.hpp"
#include "machine/Component.hpp"
#include "machine/Connection.hpp"
#include <raylib.h>
#include <raymath.h>
#include <string_view>
#ifndef CLANGD_SKIP
#define ANNOTATE(VAL) \
    [[= VAL]]
#else
#define ANNOTATE(VAL)
#endif

namespace components {

#ifndef CLANGD_SKIP
consteval std::string_view _name_of(std::meta::info r)
{
    return std::meta::identifier_of(r);
};

#endif

// template <typename T, typename M>
// inline void append_member_as_attribute(pugi::xml_node& to, const T& v, M m)
// {
// #ifndef CLANGD_SKIP
//     // constexpr auto ctx = std::meta::access_context::current();
//     constexpr auto member_ptr = ^^M;
//     constexpr auto deal = std::meta::dealias(^^M);
//     static_assert(
//             std::meta::is_member_pointer_type(member_ptr),
//             "not member_object_pointer_type");
//     auto T::*mem_ptr = m;
//     to.append_attribute(std::meta::identifier_of(^^m))
//         .set_value(v.*mem_ptr);
// #endif
// }

inline static void append_position_node(pugi::xml_node& to, const ::Vector2& pos)
{
    auto p = to.append_child("position");
    p.append_attribute("x").set_value(pos.x);
    p.append_attribute("y").set_value(pos.y);
}
inline static void append_size_attributes(pugi::xml_node& to, const ::Vector2& sz)
{
    to.append_attribute("width").set_value(sz.x);
    to.append_attribute("height").set_value(sz.y);
}

template <typename T>
struct AutoMoveD {
private:
    T val;

public:
    inline AutoMoveD(T&& v)
        : val { v }
    {
    }
    inline AutoMoveD(const T& v)
        : val { v }
    {
    }
    AutoMoveD() = default;
    AutoMoveD(const AutoMoveD&) = delete;
    AutoMoveD& operator=(const AutoMoveD&) = delete;
    AutoMoveD(AutoMoveD&&) = default;
    AutoMoveD& operator=(AutoMoveD&&) = default;
    inline T* operator->() noexcept
    {
        return &val;
    }
    inline const T* operator->() const noexcept
    {
        return &val;
    }
};
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
class OComponent : public machine::Component,
                   public game::Object,
                   public game::xml::MarshallToXml {
protected:
    ::Rectangle m_bounds { };

public:
    inline OComponent(std::string name)
        : machine::Component(name)
    {
    }
    inline OComponent(const OComponent&) = delete;
    inline OComponent& operator=(const OComponent&) = delete;
    inline OComponent(OComponent&& o)
        : machine::Component(std::move(o))
        , m_bounds { o.m_bounds }
    {
    }
    inline OComponent& operator=(OComponent&& o)
    {
        machine::Component::operator=(std::move(o));
        m_bounds = o.m_bounds;
        return *this;
    }

    inline virtual ~OComponent() { }
    inline ::Vector2 get_pos() const
    {
        return ::Vector2 { .x = m_bounds.x, .y = m_bounds.y };
    }
    virtual inline void set_pos(::Vector2 p)
    {
        this->m_bounds.x = p.x;
        this->m_bounds.y = p.y;
    }
    inline ::Vector2 get_size() const
    {
        return ::Vector2 { .x = m_bounds.width, .y = m_bounds.height };
    }
    inline ::Rectangle get_bounds() const
    {
        return m_bounds;
    }
    inline ::Vector2 get_center() const
    {
        return { m_bounds.x + m_bounds.width / 2, m_bounds.y + m_bounds.height / 2 };
    }
    inline virtual ::Vector2 get_att_point(AttachPt att) const
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
    inline virtual const char*
    marshall_to_xml_name() const noexcept override
    {
        return "";
    }
    inline virtual void
    marshall_to_xml(pugi::xml_node& self) const noexcept override
    {
        append_position_node(self, get_pos());
        self.append_attribute("name").set_value(get_name());
    }
};
class OConnection : public machine::Connection,
                    public game::Object,
                    public game::xml::MarshallToXml {
protected:
    // [[="attrib"]]
    ANNOTATE("attrib")
    ::Vector2 m_start_pos { },
        m_end_pos { };
    AttachPt m_start_ap { }, m_end_ap { };

public:
    inline OConnection(
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
    inline OConnection(const OConnection&) = delete;
    inline OConnection& operator=(const OConnection&) = delete;
    inline OConnection(OConnection&& o)
        : machine::Connection(std::move(o))
        , m_start_pos { o.m_start_pos }
        , m_end_pos { o.m_end_pos }
        , m_start_ap { o.m_start_ap }
        , m_end_ap { o.m_end_ap }
    {
    }
    inline OConnection& operator=(OConnection&& o)
    {
        machine::Connection::operator=(std::move(o));
        m_start_pos = o.m_start_pos;
        m_end_pos = o.m_end_pos;
        m_start_ap = o.m_start_ap;
        m_end_ap = o.m_end_ap;
        return *this;
    }

private:
    inline static void _set_from_attp(OConnection& self, const AttachPt& ap)
    {
        self.m_start_ap = ap;
        if (auto s = dynamic_cast<const OComponent*>(self.get_start())) {
            self.m_start_pos = s->get_att_point(ap);
        }
    }
    inline static void _set_to_attp(OConnection& self, const AttachPt& ap)
    {
        self.m_end_ap = ap;
        if (auto e = dynamic_cast<const OComponent*>(self.get_end())) {
            self.m_end_pos = e->get_att_point(ap);
        }
    }

public:
    inline virtual void set_from_attp(const AttachPt& ap)
    {
        _set_from_attp(*this, ap);
    };
    inline virtual void set_to_attp(const AttachPt& ap)
    {
        _set_to_attp(*this, ap);
    };
    inline virtual AttachPt get_from_attp()
    {
        return m_start_ap;
    };
    inline virtual AttachPt get_to_attp()
    {
        return m_end_ap;
    };

    inline virtual ~OConnection() { }
    inline virtual const char*
    marshall_to_xml_name() const noexcept override
    {
        return "";
    }
    inline virtual void
    marshall_to_xml(pugi::xml_node& self) const noexcept override
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
    }
};
}
