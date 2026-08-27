#pragma once
#include "game/Drawable.hpp"
#include "machine/Component.hpp"
#include "machine/Connection.hpp"
#include <raylib.h>
#include <raymath.h>

namespace components {
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
class OComponent : public machine::Component, public game::Object {
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
};
class OConnection : public machine::Connection, public game::Object {
protected:
    ::Vector2 m_start_pos { }, m_end_pos { };

public:
    inline OConnection(machine::Component* start,
        machine::Component* end,
        std::string in,
        std::string out)
        : machine::Connection(start, end, in, out)
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
    {
    }
    inline OConnection& operator=(OConnection&& o)
    {
        machine::Connection::operator=(std::move(o));
        m_start_pos = o.m_start_pos;
        m_end_pos = o.m_end_pos;
        return *this;
    }

private:
    inline static void _set_from_attp(OConnection& self, const AttachPt& ap)
    {
        if (auto s = dynamic_cast<const OComponent*>(self.get_start())) {
            self.m_start_pos = s->get_att_point(ap);
        }
    }
    inline static void _set_to_attp(OConnection& self, const AttachPt& ap)
    {
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

    inline virtual ~OConnection() { }
};
}
