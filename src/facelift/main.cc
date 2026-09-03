#include "components/Button.hpp"
#include "components/GameGraphElements.hpp"
#include "facelift/GatherComponents.hpp"
#include "game/Drawable.hpp"
#include "game/Scene.hpp"
#include "imgui.h"
#include "machine/MachineGraph.hpp"
#include "rlImGui.h"
#include <format>
#include <memory>
#include <optional>
#include <raylib.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

std::unordered_map<std::string, facelift::builder_fn> builders;
std::optional<std::string> open_builder { };

using either_comp_or_conn = std::variant<
    components::OComponent*, components::OConnection*>;

std::vector<either_comp_or_conn> objects { };
std::optional<
    std::pair<
        either_comp_or_conn, components::Editable*>>
    selected;

void builder_menu_draw()
{
    if (ImGui::Begin("Component builder")) {
        ImGui::BeginListBox("Components");
        for (const auto& [s, b] : builders) {
            ImGui::PushID(s.c_str());
            ImGui::Text("%s", s.c_str());
            if (ImGui::Button(std::format("Open {} menu", s).c_str())) {
                open_builder = s;
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }
    ImGui::End();
}

int main(void)
{
    ::SetConfigFlags(
        ::FLAG_WINDOW_RESIZABLE);
    ::InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Facelift");
    ::rlImGuiSetup(true);
    ::SetTargetFPS(60);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    game::resources::init_resources();
    builders = facelift::runtime_make_component_builders();
    auto mg = std::unique_ptr<machine::MachineGraph>(
        machine::MachineGraph::create());
    auto sc = game::GraphScene(std::move(mg),
        { 0,
            0,
            SCREEN_WIDTH,
            SCREEN_HEIGHT });
    while (!::WindowShouldClose()) {
        if (::IsWindowResized()) {
            auto w = ::GetScreenWidth();
            auto h = ::GetScreenHeight();
            ::SetWindowSize(w, h);
            sc.bounds.width = w;
            sc.bounds.height = h;
        }
        sc.update();
        for (auto obj : objects) {
            auto wmouse = ::GetScreenToWorld2D(::GetMousePosition(),
                *game::GraphScene::get_camera());
            const auto released = ::IsMouseButtonReleased(::MOUSE_BUTTON_LEFT);
            if (auto comp = std::get_if<components::OComponent*>(&obj); comp) {
                if (released && ::CheckCollisionPointRec(wmouse, (*comp)->get_bounds())) {
                    selected = { *comp, static_cast<components::Editable*>(*comp) };
                }
            } else {
                if (released && ::CheckCollisionPointRec(wmouse, (*comp)->get_bounds())) {
                    auto conn = std::get<components::OConnection*>(obj);
                    selected = { conn, static_cast<components::Editable*>(conn) };
                }
            }
        }
        ::BeginDrawing();
        sc.draw();
        ::rlImGuiBegin();
        builder_menu_draw();
        if (open_builder) {
            auto [c, obj] = builders[*open_builder](&sc);
            if (c)
                open_builder = std::nullopt;
            if (obj) {
                objects.push_back(*obj);
            }
        }
        if (selected) {
            bool open = true;
            ImGui::Begin("Selected", &open);
            selected->second->draw_edit_window();
            ImGui::End();
            if(!open)
                selected = std::nullopt;
        }
        ImGui::Render();
        ::rlImGuiEnd();
        ::EndDrawing();
    }
    ::rlImGuiShutdown();
    ::CloseWindow();
}

// TODO: component creation dialog, via reflecting over components namespace
// and gathering all constructor parameters
