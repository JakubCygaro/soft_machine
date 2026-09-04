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
#include <variant>

namespace facelift {
void builder_menu_draw()
{
    if (ImGui::Begin("Component builder")) {
        ImGui::BeginListBox("Components");
        for (const auto& [s, b] : fl_state.builders) {
            ImGui::PushID(s.c_str());
            ImGui::Text("%s", s.c_str());
            if (ImGui::Button(std::format("Open {} menu", s).c_str())) {
                fl_state.open_builder = s;
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }
    ImGui::End();
}

void update_objects()
{
    for (auto obj : fl_state.objects) {
        auto wmouse = ::GetScreenToWorld2D(::GetMousePosition(),
            *game::GraphScene::get_camera());
        const auto released = ::IsMouseButtonReleased(::MOUSE_BUTTON_LEFT);
        if (auto comp = std::get_if<components::OComponent*>(&obj); comp) {
            if (released && ::CheckCollisionPointRec(wmouse, (*comp)->get_bounds())) {
                fl_state.selected = { *comp,
                    static_cast<components::Editable*>(*comp) };
            }
        } else {
            if (released && ::CheckCollisionPointRec(wmouse, (*comp)->get_bounds())) {
                auto conn = std::get<components::OConnection*>(obj);
                fl_state.selected = { conn,
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
        fl_state.graph_scene->bounds.width = w;
        fl_state.graph_scene->bounds.height = h;
    }
    fl_state.graph_scene->update();
    update_objects();
}

void draw_highlight_selected_comp(components::OComponent* as_comp)
{
    ::BeginMode2D(*fl_state.graph_scene->get_instance_camera());
    ::DrawRectangleLinesEx(
        as_comp->get_bounds(),
        1.0f,
        ::RED);
    ::EndMode2D();
}

void selected_draw()
{
    bool open = true;
    components::OComponent* as_comp = nullptr;
    if (auto c = std::get_if<decltype(as_comp)>(&fl_state.selected->first);
        c) {
        as_comp = *c;
    }
    if (as_comp && ::IsKeyDown(::KEY_C)) {
        draw_highlight_selected_comp(as_comp);
    }
    ImGui::Begin("Selected", &open);
    fl_state.selected->second->draw_edit_window();
    ImGui::End();
    if (!open)
        fl_state.selected = std::nullopt;
}

void draw()
{
    ::BeginDrawing();
    fl_state.graph_scene->draw();
    ::rlImGuiBegin();
    builder_menu_draw();
    if (fl_state.open_builder) {
        auto [c, obj] = fl_state
                            .builders[*fl_state.open_builder](
                                fl_state.graph_scene.get());
        if (c)
            fl_state.open_builder = std::nullopt;
        if (obj) {
            fl_state.objects.push_back(*obj);
        }
    }
    if (fl_state.selected) {
        selected_draw();
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
    fl_state.builders = facelift::runtime_make_component_builders();
    auto mg = std::unique_ptr<machine::MachineGraph>(
        machine::MachineGraph::create());
    auto sc = game::GraphScene(std::move(mg),
        { 0,
            0,
            SCREEN_WIDTH,
            SCREEN_HEIGHT });
    fl_state.graph_scene = std::make_unique<game::GraphScene>(std::move(sc));
}
void deinit()
{
    ::rlImGuiShutdown();
    ::CloseWindow();
}
}
