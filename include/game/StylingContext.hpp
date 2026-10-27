#pragma once
#include "components/GameGraphElements.hpp"
namespace game {
class StylingContext {
public:
    virtual ~StylingContext() = default;
    virtual auto query_component(const std::string&) const
        -> std::optional<const components::OComponent&> = 0;
    virtual auto query_connection(const std::string&) const
        -> std::optional<const components::OConnection&> = 0;
};
}
