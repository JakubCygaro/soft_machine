#include "facelift/State.hpp"
#include "game/Xml.hpp"
#include "game/XmlMarshalling.hpp"
#include "imgui.h"
#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <raylib.h>
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
        return std::unexpected(
            std::format("file '{}' does not exist or is not a regular file", p));
    fl_state.filed.current_file = p;
    std::string xml;
    std::ifstream ifs(fl_state.filed.current_file);
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
std::expected<void, std::string> save_graph_to_file(const std::filesystem::path& path)
{
    pugi::xml_document doc;
    auto graph_node = doc.root()
                          .append_child("graph");
    using namespace std::views;
    auto xmls = fl_state.graph_scene
                    ->get_graph()
                    ->get_elements_as<game::xml::MarshallToXml>();
    for (const auto& elem : xmls) {
        auto node = graph_node.append_child(elem->marshall_to_xml_name());
        elem->marshall_to_xml(node);
    }
    doc.save_file(path.c_str());
    return { };
}
auto FaceliftState::FileDialog::has_file() -> bool
{
    return chosen_file.has_value();
}
auto FaceliftState::FileDialog::take_file() -> fs::path
{
    auto p = *chosen_file;
    chosen_file = std::nullopt;
    return p;
}
auto FaceliftState::FileDialog::set_cwd(std::filesystem::path p) -> void
{
    cwd = p;
    std::ranges::for_each(cwd.string() + "/\0", [&, i = 0](auto ch) mutable {
        this->input[i++] = ch;
    });
    cwd_iter = fs::directory_iterator(cwd);
}

void FaceliftState::FileDialog::draw()
{
    bool open = true;
    ImGui::Begin("File dialog", &open);
    if (last_err)
        ImGui::TextColored({ 255, 0, 0, 255 }, "%s", last_err->c_str());
    bool early_return = false;
    if (ImGui::InputText("search", this->input.data(), this->input.size())) {
        input_as_path = fs::path(input.begin(), input.end());
    }
    if (input_as_path) {
        cwd = input_as_path->parent_path();
        ImGui::SameLine();
        if (
            (::IsKeyReleased(::KEY_ENTER) || ImGui::Button("Open"))) {
            chosen_file = *input_as_path;
            input_as_path = std::nullopt;
        }
        ImGui::NewLine();
    }
    if (ImGui::BeginListBox("directory view")) {
        if (cwd.has_parent_path()) {
            const auto parent_path = cwd.parent_path();
            ImGui::PushID("../");
            if (ImGui::Selectable("../")) {
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
                    chosen_file = dir_entry.path();
                    last_err = std::nullopt;
                }
            }
            ImGui::PopID();
            if (early_return)
                break;
        }
        ImGui::EndListBox();
    }
    ImGui::End();
    if (!open)
        fl_state.file_dialog_state = FaceliftState::FileDialogState::NilState;
}
}
