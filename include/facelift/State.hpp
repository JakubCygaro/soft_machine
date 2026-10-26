#pragma once
#include "components/GameGraphElements.hpp"
#include "facelift/GatherComponents.hpp"
#include "game/Drawable.hpp"
#include <cstdint>
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
    enum class FileDialogState {
        NilState,
        OpenFile,
        SaveFile,
    } file_dialog_state = FileDialogState::NilState;

    std::unique_ptr<game::GraphScene> graph_scene = nullptr;

    components::OComponent* connect_to = nullptr;
    class FileDialog {
        std::filesystem::directory_iterator cwd_iter;
        /// what file the dialog has chosen
        std::optional<std::filesystem::path> chosen_file;
        std::array<char, 256> input { 0 };
        /// the dialog text input as a filesystem path
        std::optional<std::filesystem::path> input_as_path;

    public:
        std::optional<std::string> last_err;
        /// the path of the currently edited xml file
        std::filesystem::path current_file { };
        std::filesystem::path cwd { };
        auto draw() -> void;
        auto set_cwd(std::filesystem::path) -> void;
        auto has_file() -> bool;
        auto take_file() -> std::filesystem::path;
    } filed { };
};

inline FaceliftState fl_state;

void init(const std::vector<std::string>& args);
void update();
void draw();
void deinit();
std::expected<void, std::string> load_file(const std::string&);
std::expected<void, std::string> load_file(const std::filesystem::path&);
std::expected<void, std::string> save_graph_to_file(const std::filesystem::path&);
}
