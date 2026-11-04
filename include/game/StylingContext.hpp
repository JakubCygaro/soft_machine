#pragma once
#include "components/GameGraphElements.hpp"
#include "game/Styling.hpp"
namespace game {
class StylingContext {
public:
    virtual ~StylingContext() = default;
    virtual auto query_component(const std::string&, bool no_dep = false)
        -> std::optional<const components::OComponent&> = 0;
    virtual auto query_connection(const std::string&, bool no_dep = false)
        -> std::optional<const components::OConnection&> = 0;
    virtual auto exists(const std::string&) const -> bool = 0;
    virtual auto is_connector(const std::string&) const -> bool = 0;
    virtual auto is_component(const std::string&) const -> bool = 0;
    virtual game::StyleSchemeWrap& get_style_scheme() = 0;
};
}
