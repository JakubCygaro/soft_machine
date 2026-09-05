#include "common/String.hpp"
#include "components/Passthrough.hpp"
#include "facelift/GatherComponents.hpp"
#include "game/resources/Resources.hpp"
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace {
std::optional<int> try_parse_int(const std::string_view trimmed)
{
    int out;
    auto res = std::from_chars(
        trimmed.data(),
        trimmed.data() + trimmed.size(),
        out,
        10);
    if (res.ec == std::errc::invalid_argument) {
        return { };
    } else {
        return { out };
    }
}
}

namespace facelift {
std::unordered_map<std::string, comp_builder_fn>
runtime_make_component_builders()
{
    std::unordered_map<std::string, comp_builder_fn> ret { };
    ret["memory"] = [](game::GraphScene* s) -> comp_builder_fn::result_type {
        bool open = true;
        static bool is_ok;
        if (ImGui::Begin("memory", &open)) {
            static char name[128] = { };
            ImGui::InputText("Name", name, sizeof(name));
            static char ints[128] = { };
            ImGui::InputText("Memset", ints, sizeof(ints));
            if (ImGui::Button("Create")) {
                is_ok = false;
                auto parse = common::trim(std::string(ints));
                components::Memory::mem_t mem;
                if (parse.empty()) {
                    mem = { };
                    is_ok = true;
                } else if (auto memset = components::Memory::parse_memset_from_text(
                               parse);
                    memset.isok()) {
                    mem = *memset;
                    is_ok = true;
                } else {
                    is_ok = false;
                }
                if (!is_ok) {
                    ImGui::TextColored({ 255, 0, 0, 255 }, "Invalid memset");
                } else {
                    auto o = s->create_component<components::Memory>(
                        std::string(name), std::move(mem));
                    return std::make_pair(!open, o);
                }
            }
        }
        ImGui::End();
        return std::make_pair(!open, std::nullopt);
    };

    ret["button"] = [](game::GraphScene* s) -> comp_builder_fn::result_type {
        bool open = true;
        if (ImGui::Begin("button", &open)) {
            static char name[128] = { };
            ImGui::InputText("Name", name, sizeof(name));
            static char val[128] = { };
            ImGui::InputText("Value", val, sizeof(val));
            if (ImGui::Button("Create")) {
                std::any bval;
                auto parse = common::trim(std::string(val));
                auto trimmed = common::trim(parse);
                if (auto out = try_parse_int(std::string_view(trimmed)); out) {
                    bval = *out;
                } else {
                    bval = trimmed;
                }
                auto o = s->create_component<components::Button>(
                    std::string(name), std::move(val));
                return std::make_pair(!open, o);
            }
        }
        ImGui::End();
        return std::make_pair(!open, std::nullopt);
    };
    ret["cpu"] = [](game::GraphScene* s) -> comp_builder_fn::result_type {
        bool open = true;
        if (ImGui::Begin("cpu", &open)) {
            static char name[128] = { };
            ImGui::InputText("Name", name, sizeof(name));
            if (ImGui::Button("Create")) {
                auto o = s->create_component<components::CPU>(
                    std::string(name), components::CPU::code_t { });
                return std::make_pair(!open, o);
            }
        }
        ImGui::End();
        return std::make_pair(!open, std::nullopt);
    };
    ret["display"] = [](game::GraphScene* s) -> comp_builder_fn::result_type {
        bool open = true;
        if (ImGui::Begin("diplay", &open)) {
            static char name[128] = { };
            ImGui::InputText("Name", name, sizeof(name));
            static int font_sz = game::resources::default_node_font_size();
            ImGui::InputInt("Font size", &font_sz);
            if (ImGui::Button("Create")) {
                auto o = s->create_component<components::Display>(
                    std::string(name), font_sz);
                return std::make_pair(!open, o);
            }
        }
        ImGui::End();
        return std::make_pair(!open, std::nullopt);
    };
    return ret;
}
std::unordered_map<std::string, conn_builder_fn>
runtime_make_connection_builders()
{
    std::unordered_map<std::string, conn_builder_fn> ret { };
    ret["passthrough"] = [](
                             game::GraphScene* s,
                             const std::string& f,
                             const std::string& t)
        -> conn_builder_fn::result_type {
        bool open = true;
        static char name[128] = { };
        ImGui::InputText("Name", name, sizeof(name));
        if (ImGui::Begin("passthrough", &open)) {
            if (ImGui::Button("Connect")) {
                auto o = s->create_connection<components::Passthrough>(
                    std::string(name), f, t);
                return std::make_pair(!open, o);
            }
        }
        ImGui::End();
        return std::make_pair(!open, nullptr);
    };

    return ret;
}

}
