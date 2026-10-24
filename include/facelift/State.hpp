#pragma once
#include "components/GameGraphElements.hpp"
#include "facelift/GatherComponents.hpp"
#include "game/Drawable.hpp"
#include <expected>
#include <filesystem>
#include <memory>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

namespace facelift {
using either_comp_or_conn = std::variant<
    components::OComponent*, components::OConnection*>;
struct FaceliftState {

    std::unordered_map<
        std::string, std::unique_ptr<facelift::RuntimeComponentBuilder>>
        comp_blds;
    std::unordered_map<
        std::string, std::unique_ptr<facelift::RuntimeConnectionBuilder>>
        conn_blds;
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
    bool is_edit_mode = true;
    bool is_file_dialog_open = true;

    std::unique_ptr<game::GraphScene> graph_scene = nullptr;

    components::OComponent* connect_to = nullptr;
    class FileDialog {
        std::filesystem::directory_iterator cwd_iter;
        std::optional<std::filesystem::path> file_to_load;

    public:
        std::optional<std::string> last_err;
        std::string file_name { };
        std::filesystem::path cwd { };
        auto draw() -> void;
        auto set_cwd(std::filesystem::path) -> void;
        auto should_load_file() -> bool;
        auto get_file_to_load() -> std::filesystem::path;
    } filed { };
};

inline FaceliftState fl_state;

void init(const std::vector<std::string>& args);
void update();
void draw();
void deinit();
std::expected<void, std::string> load_file(const std::string&);
std::expected<void, std::string> load_file(const std::filesystem::path&);
}
