#include "common/String.hpp"
#include "facelift/GatherComponents.hpp"
#include <optional>
#include <string>
#include <utility>

namespace facelift {
std::unordered_map<std::string, builder_fn>
runtime_make_component_builders()
{
    std::unordered_map<std::string, builder_fn> ret { };
    ret["memory"] = [](game::GraphScene* s) -> builder_fn::result_type {
        bool open = true;
        static bool is_ok;
        if (ImGui::Begin("memory", &open)) {
            static char name[128] = { };
            ImGui::InputText("Name", name, sizeof(name));
            static char ints[128] = { };
            ImGui::InputText("Memset", ints, sizeof(name));
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

    return ret;
}

}
