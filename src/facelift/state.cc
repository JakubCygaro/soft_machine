#include "facelift/State.hpp"
#include "components/GameGraphElements.hpp"
#include "facelift/GatherComponents.hpp"
#include "game/Drawable.hpp"
#include "game/Scene.hpp"
#include "game/Styling.hpp"
#include "game/resources/Resources.hpp"
#include "imgui.h"
#include "imgui_impl_raylib.h"
#include "rlImGui.h"
#include "s7.h"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <raylib.h>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace facelift {
namespace fs = std::filesystem;
namespace {
    template <class T, class Cond, class Val>
    inline bool set_if(T& t, Cond cond, Val val)
    {
        const auto assign = [&] constexpr {
            if constexpr (std::is_invocable_v<Val>) {
                t = val();
            } else {
                t = val;
            }
            return true;
        };
        if constexpr (std::is_invocable_v<Cond>) {
            return cond() ? assign() : false;
        } else if (cond) {
            return cond ? assign() : false;
        }
        return false;
    }
    inline bool& toggle(bool& thing)
    {
        thing = !thing;
        return thing;
    }
    template <typename Func, typename L, typename R>
    void apply_on(std::variant<L, R>& var, Func fn)
    {
        if (std::holds_alternative<L>(var)) {
            fn(std::get<L>(var));
        } else if (std::holds_alternative<R>(var)) {
            fn(std::get<R>(var));
        }
    }
    // notify all relevant Editable elements related to the passed element
    void notify_element(either_comp_or_conn& element)
    {
        apply_on(element, [&](auto elem) {
            using gs_t = game::GraphScene;
            fl_state.graph_scene
                ->get_graph()
                ->style_notify_for(elem->get_name());
            fl_state.graph_scene
                ->get_graph()
                ->get_incident_to(elem->get_name())
                .and_then([&](gs_t::graph_t::incident_t* incident) {
                    for (auto* conn : *incident)
                        conn->on_notified();
                    return std::optional(Unit());
                });
        });
    }
}
void element_list_draw()
{
    ImGui::Begin("Graph elements list");
    if (ImGui::TreeNode("Components")) {
        ImGui::BeginListBox("Components");
        for (const auto& component : fl_state.graph_scene
                 ->get_graph()
                 ->get_components()) {
            ImGui::PushID(component->get_name().c_str());
            if (ImGui::Selectable(component->get_name().c_str())) {
                auto* p = component.get();
                fl_state.selected = FaceliftState::Selected {
                    .element = p,
                    .as_editable = static_cast<game::Editable*>(p),
                    .editor_object = p->get_editor_object(),
                };
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Connections")) {
        ImGui::BeginListBox("Connections");
        for (const auto& connector : fl_state.graph_scene
                 ->get_graph()
                 ->get_connections()) {
            ImGui::PushID(connector->get_name().c_str());
            if (ImGui::Selectable(connector->get_name().c_str())) {
                auto* p = connector.get();
                fl_state.selected = FaceliftState::Selected {
                    .element = p,
                    .as_editable = static_cast<game::Editable*>(p),
                    .editor_object = p->get_editor_object(),
                };
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
        ImGui::TreePop();
    }
    ImGui::End();
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
        if (ImGui::BeginListBox("Connections")) {
            for (const auto& [s, b] : fl_state.conn_blds) {
                ImGui::PushID(s.c_str());
                if (ImGui::Selectable(s.c_str())) {
                    fl_state.open_conn_bld = s;
                }
                ImGui::PopID();
            }
            ImGui::EndListBox();
        }
    }
    ImGui::End();
}

void update_objects()
{
    const auto wmouse = ::GetScreenToWorld2D(
        ::GetMousePosition(),
        *fl_state.graph_scene->get_instance_camera());
    const auto released = ::IsMouseButtonReleased(::MOUSE_BUTTON_LEFT)
        && !ImGui::GetIO().WantCaptureMouse;
    bool hit = false;
    for (auto obj : fl_state.graph_scene
             ->get_graph()
             ->get_elements_as<game::Object>()) {
        if (fl_state.is_edit_mode
            && released
            && obj->check_point_collision(wmouse)) {
            if (auto as_comp = dynamic_cast<components::OComponent*>(obj); as_comp) {
                if (fl_state.selected && ::IsKeyDown(::KEY_C)) {
                    fl_state.connect_to = as_comp;
                } else {
                    fl_state.selected = {
                        .element = as_comp,
                        .as_editable = dynamic_cast<game::Editable*>(obj),
                        .editor_object = obj->get_editor_object(),
                    };
                    fl_state.connect_to = nullptr;
                }
            } else {
                fl_state.selected = {
                    .element = dynamic_cast<components::OConnection*>(obj),
                    .as_editable = dynamic_cast<game::Editable*>(obj),
                    .editor_object = obj->get_editor_object(),
                };
                fl_state.connect_to = nullptr;
            }
            hit = true;
        }
    }
    if (!hit && fl_state.connect_to) {
        fl_state.connect_to = nullptr;
        fl_state.open_conn_bld = std::nullopt;
    }
    if (fl_state.selected
        && !fl_state.connect_to
        && ::IsKeyReleased(::KEY_DELETE)
        && !ImGui::GetIO().WantCaptureMouse) {
        apply_on(fl_state.selected->element, [&](auto& elem) {
            fl_state.graph_scene
                ->get_graph()
                ->remove_element(elem->get_name());
        });
        fl_state.selected = std::nullopt;
    }
    const auto holds_component = fl_state.selected
        && std::holds_alternative<components::OComponent*>(
            fl_state.selected->element);
    fl_state.is_dragging = fl_state.selected
        && !ImGui::GetIO().WantCaptureMouse
        && ::IsMouseButtonDown(::MOUSE_BUTTON_LEFT)
        && holds_component
        && (std::get<components::OComponent*>(fl_state.selected->element)
                ->check_point_collision(wmouse)
            || fl_state.was_dragging);
    // stopped dragging
    if (fl_state.is_edit_mode
        && fl_state.was_dragging
        && !fl_state.is_dragging
        && holds_component) {
        auto comp = std::get<components::OComponent*>(fl_state.selected->element);
        const auto b = comp->get_bounds();
        const ::Vector2 pos = {
            .x = wmouse.x - (b.width / 2.0f),
            .y = wmouse.y - (b.height / 2.0f),
        };
        comp->set_pos(pos);
        notify_element(fl_state.selected->element);
    }
    fl_state.was_dragging = fl_state.is_dragging;
    // no focus on graph if edit mode and hit
    fl_state.graph_scene
        ->set_input_to_elements(
            !(fl_state.is_edit_mode)
            && !ImGui::GetIO().WantCaptureMouse);
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
    update_objects();
    fl_state.graph_scene->update();
    if (::IsKeyDown(::KEY_LEFT_CONTROL) && ::IsKeyPressed(::KEY_E)) {
        toggle(fl_state.is_element_list_open);
    }
    if (::IsKeyDown(::KEY_LEFT_CONTROL) && ::IsKeyPressed(::KEY_W)) {
        fl_state.selected = toggle(fl_state.is_edit_mode)
            ? std::move(fl_state.selected)
            : std::nullopt;
        if (!fl_state.selected)
            fl_state.connect_to = nullptr;
    }
    if (::IsKeyReleased(::KEY_F11)) {
        toggle(fl_state.show_dump);
        if (auto dump = do_dump(); dump) {
            fl_state.graph_dump = dump.value();
        } else {
            ::TraceLog(::LOG_ERROR, "%s", dump.error().c_str());
        }
    }
    if (::IsKeyReleased(::KEY_F3)) {
        toggle(fl_state.show_lisp_errors);
    }
    using enum FaceliftState::FileDialogState;
    if (fl_state.filed.has_file() && fl_state.file_dialog_state != NilState) {
        switch (fl_state.file_dialog_state) {
        case SaveFile:
            if (auto res = save_graph_to_file(fl_state.filed.take_file()); !res) {
                ::TraceLog(::LOG_ERROR, "%s\n", res.error().c_str());
            }
            break;
        case OpenFile:
            if (auto res = load_file(fl_state.filed.take_file()); !res) {
                fl_state.filed.last_err = res.error();
                ::TraceLog(::LOG_ERROR, "%s\n", res.error().c_str());
            }
            break;
        default:
            break;
        }
        fl_state.file_dialog_state = NilState;
    }
    if (fl_state.file_dialog_state == NilState) {
        set_if(
            fl_state.file_dialog_state,
            [] {
                return ::IsKeyDown(::KEY_LEFT_CONTROL)
                    && ::IsKeyPressed(::KEY_O);
            },
            [&] {
                fl_state.filed.set_cwd(fl_state.filed.cwd);
                return OpenFile;
            });
        set_if(
            fl_state.file_dialog_state,
            [] {
                return ::IsKeyDown(::KEY_LEFT_CONTROL)
                    && ::IsKeyDown(::KEY_LEFT_SHIFT)
                    && ::IsKeyPressed(::KEY_S);
            },
            SaveFile);
    }
    if (::IsKeyDown(::KEY_LEFT_CONTROL)
        && ::IsKeyPressed(::KEY_S)
        && !::IsKeyDown(::KEY_LEFT_SHIFT)) {
        if (auto res = save_graph_to_file(fl_state.filed.current_file); !res) {
            ::TraceLog(::LOG_ERROR, "%s\n", res.error().c_str());
        }
    }
}

void draw_highlight_selected_comp(
    components::OComponent* as_comp,
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
    const auto wmouse = ::GetScreenToWorld2D(
        ::GetMousePosition(),
        *fl_state.graph_scene->get_instance_camera());
    if (auto c = std::get_if<decltype(as_comp)>(&fl_state.selected.value().element);
        c) {
        as_comp = *c;
    }
    if (as_comp) {
        draw_highlight_selected_comp(as_comp, ::RED);
        if (fl_state.connect_to) {
            draw_highlight_selected_comp(fl_state.connect_to, ::BLUE);
            conn_builder_menu_draw();
        }
        // is still dragging
        if (fl_state.was_dragging
            && fl_state.is_dragging
            && fl_state.is_edit_mode) {
            auto comp = std::get<components::OComponent*>(fl_state.selected->element);
            const auto b = comp->get_bounds();
            const ::Vector2 pos = {
                .x = wmouse.x - (b.width / 2.0f),
                .y = wmouse.y - (b.height / 2.0f),
            };
            ::BeginMode2D(*fl_state.graph_scene->get_instance_camera());
            ::DrawRectangleLinesEx(
                ::Rectangle {
                    .x = pos.x,
                    .y = pos.y,
                    .width = b.width,
                    .height = b.height,
                },
                5.0f,
                ::RED);
            ::EndMode2D();
        }
    }
    ImGui::Begin("Selected", &open);
    if (fl_state.selected) {
        fl_state.selected->editor_object->draw_update()
            ? notify_element(fl_state.selected->element)
            : (void)0;
    }
    ImGui::End();
    if (!open)
        fl_state.selected = std::nullopt;
}
void lisp_error_draw()
{
    if (ImGui::Begin("Lisp errors", &fl_state.show_lisp_errors)) {
        ImGui::Text("%s", fl_state.lisp_error_log.c_str());
    }
    ImGui::End();
}

void dump_draw()
{
    if (ImGui::Begin("Graph Dump", &fl_state.show_dump)) {
        ImGui::Text("%s", fl_state.graph_dump.value().c_str());
    }
    ImGui::End();
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
        auto& bld = fl_state.comp_blds[*fl_state.open_comp_bld];
        auto obj = bld->display_builder(
            fl_state.graph_scene.get());
        if (bld->is_should_close())
            fl_state.open_comp_bld = std::nullopt;
        if (obj) {
            fl_state.open_comp_bld = std::nullopt;
        }
    }
    if (fl_state.open_conn_bld) {
        auto& from = std::get<components::OComponent*>(fl_state.selected->element)
                         ->get_name();
        auto& to = fl_state.connect_to->get_name();
        auto& bld = fl_state.conn_blds[*fl_state.open_conn_bld];
        auto obj = bld->display_builder(
            fl_state.graph_scene.get(),
            from,
            to);
        if (bld->is_should_close() || obj) {
            fl_state.open_conn_bld = std::nullopt;
            fl_state.selected = std::nullopt;
        }
    }
    if (fl_state.show_dump) {
        dump_draw();
    }
    if (fl_state.show_lisp_errors) {
        lisp_error_draw();
    }
    if (fl_state.selected) {
        selected_draw();
    }
    if (fl_state.is_element_list_open)
        element_list_draw();
    if (fl_state.file_dialog_state != FaceliftState::FileDialogState::NilState)
        fl_state.filed.draw();
    // ::rlImGuiEnd();
    ImGui::Render();
    ::ImGui_ImplRaylib_RenderDrawData(ImGui::GetDrawData());
    ::EndDrawing();
}

extern "C" {
static void lisp_style_error_port(::s7_scheme*, uint8_t c, ::s7_pointer)
{
    auto& log = fl_state.lisp_error_log;
    if (log.size() >= 512) {
        std::shift_left(
            log.begin(),
            log.end(),
            100);
        log.clear();
    }
    log.push_back(static_cast<char>(c));
}
}

void init(const std::vector<std::string>& args)
{
    if (args.size() > 1) {
        throw std::runtime_error("too many command line arguments passed");
    }
    ::SetConfigFlags(
        ::FLAG_WINDOW_RESIZABLE);
    ::InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Facelift");
#ifdef DEBUG
    ::SetTraceLogLevel(::LOG_DEBUG);
#endif
    ::rlImGuiBeginInitImGui();
    ::SetTargetFPS(60);

    IMGUI_CHECKVERSION();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ::rlImGuiEndInitImGui();

    fl_state.filed.set_cwd(fs::current_path());
    game::resources::init_resources();
    fl_state.comp_blds = facelift::runtime_make_component_builders();
    fl_state.conn_blds = facelift::runtime_make_connection_builders();
    auto bld = game::StyleBuilder();
    bld.with_font(::GetFontDefault())
        .with_font_data(
            { .size = 24,
                .spacing = 10 });
    auto def_style = std::move(bld)
                         .build_style();
    auto mg = std::unique_ptr<game::GraphScene::graph_t>(
        game::GraphScene::graph_t::create(
            std::make_shared<game::Style>(std::move(def_style))));
    auto sc = game::GraphScene(std::move(mg),
        { 0,
            0,
            SCREEN_WIDTH,
            SCREEN_HEIGHT });
    fl_state.graph_scene = std::make_unique<game::GraphScene>(std::move(sc));
    fl_state.lisp_error_log.reserve(256);
    fl_state.graph_scene
        ->get_graph()
        ->get_style_scheme()
        .set_error_port(&lisp_style_error_port);

    if (args.size() == 1) {
        auto p = std::string(args[0]);
        auto res = load_file(p);
        if (!res) {
            ::TraceLog(::LOG_ERROR, "Failed to load file, %s", res.error().c_str());
        }
    }
}
void deinit()
{
    ::rlImGuiShutdown();
    ::CloseWindow();
}
}
