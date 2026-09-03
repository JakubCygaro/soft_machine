#include "components/Button.hpp"
#include "facelift/GatherComponents.hpp"
#include "game/Scene.hpp"
#include "imgui.h"
#include "machine/MachineGraph.hpp"
#include "misc/cpp/imgui_stdlib.h"
#include "rlImGui.h"
#include <format>
#include <memory>
#include <raylib.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

std::unordered_map<std::string, facelift::builder_fn> builders;
std::optional<std::string> open_builder { };

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
        ::BeginDrawing();
        sc.draw();
        ::rlImGuiBegin();
        builder_menu_draw();
        if (open_builder) {
            if (builders[*open_builder](&sc))
                open_builder = std::nullopt;
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
