#pragma once
#include "game/Drawable.hpp"
#include "machine/Component.hpp"
#include "machine/Connection.hpp"
#include <raylib.h>

namespace components {
class OComponent : public machine::Component, public game::Object {
protected:
    ::Rectangle m_bounds { };
};
class OConnection : public machine::Connection, public game::Object {
protected:
    ::Vector2 m_start_pos { }, m_end_pos { };
};
}
