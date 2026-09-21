#include "facelift/State.hpp"
#include "components/GameGraphElements.hpp"
#include "facelift/GatherComponents.hpp"
#include "game/Drawable.hpp"
#include "game/Scene.hpp"
#include "game/Xml.hpp"
#include "imgui.h"
#include "imgui_impl_raylib.h"
#include "rlImGui.h"
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <raylib.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace facelift {
namespace {
    template <typename Func, typename L, typename R>
    void apply_on(std::variant<L, R>& var, Func fn)
    {
        if (std::holds_alternative<L>(var)) {
            fn(std::get<L>(var));
        } else if (std::holds_alternative<R>(var)) {
            fn(std::get<R>(var));
        }
    }
}
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
    const auto wmouse = ::GetScreenToWorld2D(
        ::GetMousePosition(),
        *game::GraphScene::get_camera());
    const auto released = ::IsMouseButtonReleased(::MOUSE_BUTTON_LEFT)
        && !ImGui::GetIO().WantCaptureMouse;
    bool hit = true;
    for (auto obj : fl_state.graph_scene
             ->get_graph()
             ->get_elements_as<game::Object>()) {
        if (released && obj->check_point_collision(wmouse)) {
            if (auto as_comp = dynamic_cast<components::OComponent*>(obj); as_comp) {
                if (fl_state.selected && ::IsKeyDown(::KEY_C)) {
                    fl_state.connect_to = as_comp;
                } else {
                    fl_state.selected = { as_comp, dynamic_cast<game::Editable*>(obj) };
                    fl_state.connect_to = nullptr;
                }
            } else {
                fl_state.selected = { dynamic_cast<components::OConnection*>(obj),
                    dynamic_cast<game::Editable*>(obj) };
                fl_state.connect_to = nullptr;
            }
            hit = true;
        }
    }
    // for (auto obj : fl_state.objects) {
    //     auto wmouse = ::GetScreenToWorld2D(::GetMousePosition(),
    //         *game::GraphScene::get_camera());
    //     const auto released = ::IsMouseButtonReleased(::MOUSE_BUTTON_LEFT)
    //         && !ImGui::GetIO().WantCaptureMouse;
    //     hit = !released;
    //     using namespace components;
    //     if (auto comp = std::get_if<OComponent*>(&obj); comp) {
    //         if (released && ::CheckCollisionPointRec(wmouse, (*comp)->get_bounds())) {
    //             if (fl_state.selected && ::IsKeyDown(::KEY_C)) {
    //                 fl_state.connect_to = *comp;
    //             } else {
    //                 fl_state.selected = { *comp,
    //                     static_cast<Editable*>(*comp) };
    //                 fl_state.connect_to = nullptr;
    //             }
    //             hit = true;
    //         }
    //     } else if (auto* conn = std::get<OConnection*>(obj); released
    //         && ::CheckCollisionPointLine(
    //             wmouse,
    //             conn->get_start_pos(),
    //             conn->get_end_pos(),
    //             5)) {
    //         fl_state.selected = { conn,
    //             static_cast<Editable*>(conn) };
    //         hit = true;
    //     }
    // }
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
    if (auto c = std::get_if<decltype(as_comp)>(&fl_state.selected.value().first);
        c) {
        as_comp = *c;
    }
    if (as_comp) {
        draw_highlight_selected_comp(as_comp, ::RED);
        if (fl_state.connect_to) {
            draw_highlight_selected_comp(fl_state.connect_to, ::BLUE);
            conn_builder_menu_draw();
        }
    }
    ImGui::Begin("Selected", &open);
    if (fl_state.selected) {
        fl_state.selected->second->draw_edit_window()
            ? apply_on(fl_state.selected->first, [&](auto elem) {
                  fl_state.graph_scene
                      ->get_graph()
                      ->notify(elem->get_name());
              })
            : (void)0;
    }
    ImGui::End();
    if (!open)
        fl_state.selected = std::nullopt;
}

void draw()
{
    // ::rlImGuiBegin();
    const auto& io = ImGui::GetIO();
    fl_state.graph_scene->set_focus(
        !(io.WantCaptureMouse));
    ::ImGui_ImplRaylib_ProcessEvents();
    ::ImGui_ImplRaylib_NewFrame();
    ImGui::NewFrame();

    ::BeginDrawing();
    ::ClearBackground(::BLACK);

    fl_state.graph_scene->draw();
    comp_builder_menu_draw();
    if (fl_state.open_comp_bld) {
        auto [c, obj] = fl_state
                            .comp_blds[*fl_state.open_comp_bld](
                                fl_state.graph_scene.get());
        if (c)
            fl_state.open_comp_bld = std::nullopt;
        if (obj) {
            fl_state.open_comp_bld = std::nullopt;
            // fl_state.objects.push_back(*obj);
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
        // if (obj) {
        //     fl_state.objects.push_back(*obj);
        // }
    }
    if (fl_state.selected) {
        selected_draw();
    }
    // ::rlImGuiEnd();
    ImGui::Render();
    ::ImGui_ImplRaylib_RenderDrawData(ImGui::GetDrawData());
    ::EndDrawing();
}

void init(const std::vector<std::string>& args)
{
    if (args.size() > 1) {
        throw std::runtime_error("too many command line arguments passed");
    }
    ::SetConfigFlags(
        ::FLAG_WINDOW_RESIZABLE);
    ::InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Facelift");
    ::rlImGuiBeginInitImGui();
    ::SetTargetFPS(60);

    IMGUI_CHECKVERSION();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ::rlImGuiEndInitImGui();

    game::resources::init_resources();
    fl_state.comp_blds = facelift::runtime_make_component_builders();
    fl_state.conn_blds = facelift::runtime_make_connection_builders();

    auto mg = std::unique_ptr<game::GraphScene::graph_t>(
        game::GraphScene::graph_t::create());
    if (args.size() == 1) {
        std::string xml;
        std::ifstream ifs(args[0]);
        std::stringstream buf;
        buf << ifs.rdbuf();
        xml = buf.str();
        auto res = game::populate_machine_from_xml(*mg, xml);
        if (res.iserr()) {
            throw res.unwrap_err();
        }
    }

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
