#include "common/String.hpp"
#include "components/Passthrough.hpp"
#include "facelift/GatherComponents.hpp"
#include "game/resources/Resources.hpp"
#include <concepts>
#include <memory>
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
bool BuilderBase::is_should_close() noexcept
{
    auto ret = should_close;
    should_close = should_close ? false
                                : should_close;
    return ret;
}
void BuilderBase::set_should_close() noexcept
{
    should_close = true;
}
class MemoryBuilder : public ComponentBuilder<components::Memory> {
public:
    virtual const std::string& get_element_name() override
    {
        static std::string n = "memory";
        return n;
    }
    comp_t*
    display_builder(game::GraphScene* s) override
    {
        bool open = true;
        if (ImGui::Begin(get_element_name().c_str(), &open)) {
            static char name[128] = { };
            ImGui::InputText("Name", name, sizeof(name));
            static char ints[128] = { };
            ImGui::InputText("Memset", ints, sizeof(ints));
            if (ImGui::Button("Create")) {
                auto parse = common::trim(std::string(ints));
                components::Memory::mem_t mem;
                if (parse.empty()) {
                    mem = { };
                } else if (auto memset = components::Memory::parse_memset_from_text(
                               parse);
                    memset.isok()) {
                    mem = *memset;
                } else {
                    last_error = memset.unwrap_err();
                }
                if (last_error) {
                    ImGui::TextColored({ 255, 0, 0, 255 }, "%s", last_error->what());
                }
                try {
                    auto o = s->create_component<components::Memory>(
                        std::string(name), std::move(mem));
                    ImGui::End();
                    last_error = std::nullopt;
                    return o;
                } catch (std::runtime_error& e) {
                    last_error = std::move(e);
                }
            }
        }
        ImGui::End();
        if (!open)
            set_should_close();
        return nullptr;
    }
    virtual ~MemoryBuilder() = default;
};
class ButtonBuilder : public ComponentBuilder<components::Button> {
public:
    virtual const std::string& get_element_name() override
    {
        static std::string n = "button";
        return n;
    }
    comp_t*
    display_builder(game::GraphScene* s) override
    {
        bool open = true;
        if (ImGui::Begin(get_element_name().c_str(), &open)) {
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
                if (last_error) {
                    ImGui::TextColored({ 255, 0, 0, 255 }, "%s", last_error->what());
                }
                try {
                    auto o = s->create_component<components::Button>(
                        std::string(name), std::move(val));
                    ImGui::End();
                    last_error = std::nullopt;
                    return o;
                } catch (std::runtime_error& e) {
                    last_error = std::move(e);
                }
                ImGui::End();
            }
        }
        ImGui::End();
        if (!open)
            set_should_close();
        return nullptr;
    }
    virtual ~ButtonBuilder() = default;
};
class CPUBuilder : public ComponentBuilder<components::CPU> {
public:
    virtual const std::string& get_element_name() override
    {
        static std::string n = "cpu";
        return n;
    }
    comp_t*
    display_builder(game::GraphScene* s) override
    {
        bool open = true;
        if (ImGui::Begin(get_element_name().c_str(), &open)) {
            static char name[128] = { };
            ImGui::InputText("Name", name, sizeof(name));
            if (last_error) {
                ImGui::TextColored({ 255, 0, 0, 255 }, "%s", last_error->what());
            }
            if (ImGui::Button("Create")) {
                try {
                    auto o = s->create_component<components::CPU>(
                        std::string(name), components::CPU::code_t { });
                    ImGui::End();
                    last_error = { };
                    return o;
                } catch (std::runtime_error& e) {
                    last_error = std::move(e);
                }
            }
        }
        ImGui::End();
        if (!open)
            set_should_close();
        return nullptr;
    }
    virtual ~CPUBuilder() = default;
};
class DisplayBuilder : public ComponentBuilder<components::Display> {
public:
    virtual const std::string& get_element_name() override
    {
        static std::string n = "display";
        return n;
    }
    comp_t*
    display_builder(game::GraphScene* s) override
    {
        bool open = true;
        if (ImGui::Begin(get_element_name().c_str(), &open)) {
            static char name[128] = { };
            ImGui::InputText("Name", name, sizeof(name));
            static int font_sz = game::resources::default_node_font_size();
            ImGui::InputInt("Font size", &font_sz);
            if (last_error) {
                ImGui::TextColored({ 255, 0, 0, 255 }, "%s", last_error->what());
            }
            if (ImGui::Button("Create")) {
                try {
                    auto o = s->create_component<components::Display>(
                        std::string(name), font_sz);
                    ImGui::End();
                    last_error = { };
                    return o;
                } catch (std::runtime_error& e) {
                    last_error = std::move(e);
                }
            }
        }
        ImGui::End();
        if (!open)
            set_should_close();
        return nullptr;
    }
    virtual ~DisplayBuilder() = default;
};

std::unordered_map<std::string, std::unique_ptr<RuntimeComponentBuilder>>
runtime_make_component_builders()
{
    std::unordered_map<
        std::string,
        std::unique_ptr<RuntimeComponentBuilder>>
        ret { };
    const auto add_comp = [&]<std::derived_from<RuntimeComponentBuilder> T>() {
        auto c = std::make_unique<T>();
        ret[c->get_element_name()] = std::move(c);
    };
    add_comp.operator()<MemoryBuilder>();
    add_comp.operator()<ButtonBuilder>();
    add_comp.operator()<CPUBuilder>();
    add_comp.operator()<DisplayBuilder>();
    return ret;
}

class PassthroughBuilder : public ConnectionBuilder<components::Passthrough> {
public:
    virtual const std::string& get_element_name() override
    {
        static std::string n = "passthrough";
        return n;
    }
    conn_t*
    display_builder(game::GraphScene* s,
        const std::string& f,
        const std::string& t) override
    {
        bool open = true;
        if (ImGui::Begin(get_element_name().c_str(), &open)) {
            static char name[128] = { };
            ImGui::InputText("Name", name, sizeof(name));
            if (ImGui::Button("Connect")) {
                if (last_error) {
                    ImGui::TextColored({ 255, 0, 0, 255 }, "%s", last_error->what());
                }
                try {
                    auto o = s->create_connection<components::Passthrough>(
                        std::string(name),
                        f,
                        t);
                    ImGui::End();
                    return o;
                } catch (std::runtime_error& e) {
                    last_error = std::move(e);
                }
            }
        }
        ImGui::End();
        if (!open)
            set_should_close();
        return nullptr;
    }
    virtual ~PassthroughBuilder() = default;
};
std::unordered_map<std::string, std::unique_ptr<RuntimeConnectionBuilder>>
runtime_make_connection_builders()
{
    auto ret = std::unordered_map<
        std::string,
        std::unique_ptr<RuntimeConnectionBuilder>>();
    const auto add_conn = [&]<std::derived_from<RuntimeConnectionBuilder> T>() {
        auto c = std::make_unique<T>();
        ret[c->get_element_name()] = std::move(c);
    };
    add_conn.operator()<PassthroughBuilder>();
    return ret;
}
}
