#include "components/Button.hpp"
#include "facelift/GatherComponents.hpp"
#include "game/Scene.hpp"
#include "imgui.h"
#include "machine/MachineGraph.hpp"
#include "rlImGui.h"
#include <memory>
#include <raylib.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

std::unordered_map<std::string, facelift::builder_fn> builders;
std::optional<std::string> open_builder { };

void builder_menu_draw()
{
    static bool open_builders = false;
    ImGui::Begin("Chooj", &open_builders, 0);
    ImGui::BeginListBox("Components");
    for (const auto& [s, b] : builders) {
        ImGui::PushID(s.c_str());
        ImGui::Text("%s", s.c_str());
        if (ImGui::Button("Open menu")) {
            open_builder = s;
            ImGui::PopID();
            break;
        }
        ImGui::PopID();
    }
    ImGui::EndListBox();
    ImGui::End();
}

int main(void)
{
    ::SetConfigFlags(
        ::FLAG_WINDOW_RESIZABLE);
    ::InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Facelift");
    ::rlImGuiSetup(true);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    builders = facelift::make_component_builders();
    builders["button"] = [](game::GraphScene* s) {
        bool close = false;
        ImGui::Begin("button", &close);
        static char name_buf[128] = { };
        ImGui::InputText("Name", name_buf, sizeof(name_buf));
        static int val { };
        ImGui::InputInt("Val", &val);
        if (ImGui::Button("create")) {
            s->create_component<components::Button>(
                std::string(name_buf), val);
        }
        ImGui::End();
    };

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
            builders[*open_builder](&sc);
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
