#include "facelift/State.hpp"
#include "game/Xml.hpp"
#include "imgui.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
namespace facelift {
namespace fs = std::filesystem;
std::expected<void, std::string> load_file(const std::string& p)
{
    auto path = fs::path(p);
    return load_file(path);
}
std::expected<void, std::string> load_file(const std::filesystem::path& p)
{
    if (!fs::exists(p) || !fs::is_regular_file(p))
        throw std::unexpected(
            std::format("file '{}' does not exist or is not a regular file", p));
    fl_state.filed.file_name = p;
    std::string xml;
    std::ifstream ifs(fl_state.filed.file_name);
    std::stringstream buf;
    buf << ifs.rdbuf();
    xml = buf.str();
    auto modify = [&](game::GraphScene::graph_t* g) -> auto {
        g->clear();
        auto res = game::populate_machine_from_xml(*g, xml);
        return res;
    };
    auto res = fl_state.graph_scene
                   ->modify_graph<Result<std::runtime_error, Unit>>(modify);
    if (res.iserr()) {
        fl_state.graph_scene->get_graph()->clear();
        return std::unexpected(res.unwrap_err().what());
    }
    return { };
}
auto FaceliftState::FileDialog::set_cwd(std::filesystem::path p) -> void
{
    cwd = p;
    cwd_iter = fs::directory_iterator(cwd);
}
void FaceliftState::FileDialog::draw()
{
    ImGui::Begin("File dialog", &fl_state.is_file_dialog_open);
    if (last_err)
        ImGui::TextColored({ 255, 0, 0, 255 }, "%s", last_err->c_str());
    bool early_return = false;
    if (ImGui::BeginListBox("File dialog")) {
        if (cwd.has_parent_path()) {
            auto parent_path = cwd.parent_path();
            ImGui::PushID("..");
            if (ImGui::Selectable("..")) {
                cwd = parent_path;
                early_return = true;
            }
            ImGui::PopID();
        }
        for (auto const& dir_entry : fs::directory_iterator(cwd)) {
            if (!dir_entry.is_directory() && !dir_entry.is_regular_file())
                continue;

            const auto& name = dir_entry.path().filename();
            ImGui::PushID(name.c_str());
            const auto display_name = std::format("{}{}", name.c_str(),
                dir_entry.is_directory() ? "/" : "");
            if (ImGui::Selectable(display_name.c_str())) {
                if (dir_entry.is_directory()) {
                    cwd = dir_entry.path();
                    cwd_iter = fs::directory_iterator(cwd);
                    early_return = true;
                } else {
                    auto res = load_file(dir_entry.path());
                    if (!res) {
                        last_err = res.error();
                    } else {
                        last_err = std::nullopt;
                    }
                }
            }
            ImGui::PopID();
            if (early_return)
                break;
        }
        ImGui::EndListBox();
    }
    ImGui::End();
}
}
