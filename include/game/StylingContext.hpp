#pragma once
#include "components/GameGraphElements.hpp"
#include "game/Styling.hpp"
namespace game {
class StylingContext {
public:
    virtual ~StylingContext() = default;
    virtual auto query_component(const std::string&) const
        -> std::optional<const components::OComponent&> = 0;
    virtual auto query_connection(const std::string&) const
        -> std::optional<const components::OConnection&> = 0;
    virtual game::StyleSchemeWrap& get_style_scheme() = 0;
};
}
