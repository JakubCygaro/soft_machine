#pragma once
#include "components/GameGraphElements.hpp"
#include "facelift/GatherComponents.hpp"
#include "game/Drawable.hpp"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

namespace facelift {
using either_comp_or_conn = std::variant<
    components::OComponent*, components::OConnection*>;
struct FaceliftState {

    std::unordered_map<std::string, facelift::comp_builder_fn> comp_blds;
    std::unordered_map<std::string, facelift::conn_builder_fn> conn_blds;
    std::optional<std::string> open_comp_bld { };
    std::optional<std::string> open_conn_bld { };

    // std::vector<either_comp_or_conn> objects { };
    std::optional<
        std::pair<
            either_comp_or_conn, game::Editable*>>
        selected;
    bool is_dragging = false;
    bool was_dragging = false;
    bool is_element_list_open = false;

    std::unique_ptr<game::GraphScene> graph_scene = nullptr;

    //
    components::OComponent* connect_to = nullptr;
};

inline static FaceliftState fl_state;

void init(const std::vector<std::string>& args);
void update();
void draw();
void deinit();
}
