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
void comp_builder_menu_draw()
{
    if (ImGui::Begin("Component builder")) {
        ImGui::BeginListBox("Components");
        for (const auto& [s, b] : fl_state.comp_blds) {
            ImGui::PushID(s.c_str());
            if (ImGui::Selectable(s.c_str())) {
                fl_state.open_comp_bld = s;
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }
    ImGui::End();
}
void conn_builder_menu_draw()
{
    if (ImGui::Begin("Connection builder")) {
        ImGui::BeginListBox("Connections");
        for (const auto& [s, b] : fl_state.conn_blds) {
            ImGui::PushID(s.c_str());
            if (ImGui::Selectable(s.c_str())) {
                fl_state.open_conn_bld = s;
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }
    ImGui::End();
}

void update_objects()
{
    bool hit = true;
    for (auto obj : fl_state.objects) {
        auto wmouse = ::GetScreenToWorld2D(::GetMousePosition(),
            *game::GraphScene::get_camera());
        const auto released = ::IsMouseButtonReleased(::MOUSE_BUTTON_LEFT);
        hit = !released;
        using namespace components;
        if (auto comp = std::get_if<OComponent*>(&obj); comp) {
            if (released && ::CheckCollisionPointRec(wmouse, (*comp)->get_bounds())) {
                if (fl_state.selected && ::IsKeyDown(::KEY_C)) {
                    fl_state.connect_to = *comp;
                } else {
                    fl_state.selected = { *comp,
                        static_cast<Editable*>(*comp) };
                    fl_state.connect_to = nullptr;
                }
                hit = true;
            }
        } else if (auto* conn = std::get<OConnection*>(obj); released
            && ::CheckCollisionPointLine(
                wmouse,
                conn->get_start_pos(),
                conn->get_end_pos(),
                5)) {
            fl_state.selected = { conn,
                static_cast<Editable*>(conn) };
            hit = true;
        }
    }
    if (!hit && fl_state.connect_to) {
        fl_state.connect_to = nullptr;
        fl_state.open_conn_bld = std::nullopt;
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

void draw_highlight_selected_comp(components::OComponent* as_comp,
    ::Color color)
{
    ::BeginMode2D(*fl_state.graph_scene->get_instance_camera());
    ::DrawRectangleLinesEx(
        as_comp->get_bounds(),
        1.0f,
        color);
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
        draw_highlight_selected_comp(as_comp, ::RED);
        if (fl_state.connect_to) {
            draw_highlight_selected_comp(fl_state.connect_to, ::BLUE);
            conn_builder_menu_draw();
        }
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
    comp_builder_menu_draw();
    if (fl_state.open_comp_bld) {
        auto [c, obj] = fl_state
                            .comp_blds[*fl_state.open_comp_bld](
                                fl_state.graph_scene.get());
        if (c)
            fl_state.open_comp_bld = std::nullopt;
        if (obj) {
            fl_state.objects.push_back(*obj);
        }
    }
    if (fl_state.open_conn_bld) {
        auto& from = std::get<components::OComponent*>(fl_state.selected->first)
                         ->get_name();
        auto& to = fl_state.connect_to->get_name();
        auto [c, obj] = fl_state
                            .conn_blds[*fl_state.open_conn_bld](
                                fl_state.graph_scene.get(),
                                from,
                                to);
        if (c)
            fl_state.open_conn_bld = std::nullopt;
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
    fl_state.comp_blds = facelift::runtime_make_component_builders();
    fl_state.conn_blds = facelift::runtime_make_connection_builders();
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
