#include "facelift/State.hpp"
#include "components/GameGraphElements.hpp"
#include "facelift/GatherComponents.hpp"
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

namespace facelift {
void builder_menu_draw()
{
    if (ImGui::Begin("Component builder")) {
        ImGui::BeginListBox("Components");
        for (const auto& [s, b] : facelift_state.builders) {
            ImGui::PushID(s.c_str());
            ImGui::Text("%s", s.c_str());
            if (ImGui::Button(std::format("Open {} menu", s).c_str())) {
                facelift_state.open_builder = s;
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }
    ImGui::End();
}

void update_objects()
{
    for (auto obj : facelift_state.objects) {
        auto wmouse = ::GetScreenToWorld2D(::GetMousePosition(),
            *game::GraphScene::get_camera());
        const auto released = ::IsMouseButtonReleased(::MOUSE_BUTTON_LEFT);
        if (auto comp = std::get_if<components::OComponent*>(&obj); comp) {
            if (released && ::CheckCollisionPointRec(wmouse, (*comp)->get_bounds())) {
                facelift_state.selected = { *comp,
                    static_cast<components::Editable*>(*comp) };
            }
        } else {
            if (released && ::CheckCollisionPointRec(wmouse, (*comp)->get_bounds())) {
                auto conn = std::get<components::OConnection*>(obj);
                facelift_state.selected = { conn,
                    static_cast<components::Editable*>(conn) };
            }
        }
    }
}
void update()
{
    if (::IsWindowResized()) {
        auto w = ::GetScreenWidth();
        auto h = ::GetScreenHeight();
        ::SetWindowSize(w, h);
        facelift_state.graph_scene->bounds.width = w;
        facelift_state.graph_scene->bounds.height = h;
    }
    facelift_state.graph_scene->update();
    update_objects();
}

void draw()
{
    ::BeginDrawing();
    facelift_state.graph_scene->draw();
    ::rlImGuiBegin();
    builder_menu_draw();
    if (facelift_state.open_builder) {
        auto [c, obj] = facelift_state
                            .builders[*facelift_state.open_builder](
                                facelift_state.graph_scene.get());
        if (c)
            facelift_state.open_builder = std::nullopt;
        if (obj) {
            facelift_state.objects.push_back(*obj);
        }
    }
    if (facelift_state.selected) {
        bool open = true;
        ImGui::Begin("Selected", &open);
        facelift_state.selected->second->draw_edit_window();
        ImGui::End();
        if (!open)
            facelift_state.selected = std::nullopt;
    }
    ImGui::Render();
    ::rlImGuiEnd();
    ::EndDrawing();
}

void init()
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
    facelift_state.builders = facelift::runtime_make_component_builders();
    auto mg = std::unique_ptr<machine::MachineGraph>(
        machine::MachineGraph::create());
    auto sc = game::GraphScene(std::move(mg),
        { 0,
            0,
            SCREEN_WIDTH,
            SCREEN_HEIGHT });
    facelift_state.graph_scene = std::make_unique<game::GraphScene>(std::move(sc));
}
void deinit()
{
    ::rlImGuiShutdown();
    ::CloseWindow();
}
}
