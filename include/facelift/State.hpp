#pragma once
#include "components/GameGraphElements.hpp"
#include "facelift/GatherComponents.hpp"

namespace facelift {
using either_comp_or_conn = std::variant<
    components::OComponent*, components::OConnection*>;
struct FaceliftState {

    std::unordered_map<std::string, facelift::builder_fn> builders;
    std::optional<std::string> open_builder { };

    std::vector<either_comp_or_conn> objects { };
    std::optional<
        std::pair<
            either_comp_or_conn, components::Editable*>>
        selected;

    std::unique_ptr<game::GraphScene> graph_scene = nullptr;
};

inline static FaceliftState facelift_state;

void init();
void update();
void draw();
void deinit();
}
