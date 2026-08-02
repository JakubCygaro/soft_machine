#include "game/resources/Resources.hpp"
#include <raylib.h>
#include <stdexcept>

namespace {

static ::Font _node_font;

static bool _initialized = false;

}

namespace game::resources {

::Font get_node_font()
{
    return _node_font;
}
void init_resources()
{
    if (_initialized)
        throw std::runtime_error(
            "init_resources() attempted to reinitialize game resources");

    _node_font = ::GetFontDefault();

    _initialized = true;
}

void deinit_resources()
{
    if (!_initialized)
        throw std::runtime_error(
            "deinit_resources() resources have not been initialized");
}

}
