#pragma once
#include "components/Memory.hpp"
#include "facelift/Fl.hpp"
#include "game/Scene.hpp"
#include "imgui.h"
#include "machine/MachineGraph.hpp"
#include <array>
#include <format>
#include <functional>
#include <iostream>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#ifndef CLANGD_SKIP
#include <meta>
#endif

namespace facelift {

using builder_fn = std::function<void(game::GraphScene*)>;

#ifndef CLANGD_SKIP
using namespace std::meta;
consteval bool is_component(std::meta::info c)
{
    if (!is_type(c))
        return false;
    auto anns = std::define_static_array(annotations_of(c));
    for (auto a : anns) {
        if (type_of(a) == type_of(^^fl::component))
            return true;
    }
    return false;
}
consteval auto has_build_ctor(std::meta::info c) -> bool
{
    auto ctx = std::meta::access_context::current();
    auto mems = std::define_static_array(members_of(c, ctx));
    std::optional<std::meta::info> ret;
    bool found = false;
    for (auto mem : mems) {
        if (!is_constructor(mem))
            continue;
        auto anns = std::define_static_array(annotations_of(mem));
        for (auto a : anns) {
            if (type_of(a) == type_of(^^fl::use_ctor) && !found) {
                ret = mem;
                found = true;
            } else if (found) {
                return false;
            }
        }
    }
    return found;
}

consteval auto get_build_ctor(std::meta::info c) -> std::meta::info
{
    auto ctx = std::meta::access_context::current();
    auto mems = std::define_static_array(members_of(c, ctx));
    for (auto mem : mems) {
        if (!is_constructor(mem))
            continue;
        auto anns = std::define_static_array(annotations_of(mem));
        for (auto a : anns) {
            if (type_of(a) == type_of(^^fl::use_ctor)) {
                return mem;
            }
        }
    }
}
consteval auto get_components() -> auto
{
    auto ctx = access_context::current();
    std::vector<info> comps { };
    for (auto c : std::define_static_array(members_of(^^::components, ctx))) {
        if (is_component(c)) {
            comps.push_back(c);
        }
    }
    return comps;
}

consteval auto is_ignore(std::meta::info param) -> bool
{
    if (std::ranges::any_of(annotations_of(param), [](auto a) {
            return type_of(a) == type_of(^^fl::ignore);
        })) {
        return false;
    }
    return false;
}
consteval auto get_params(info ctor) -> auto
{
    std::vector<info> ret { };
    for (auto p : parameters_of(ctor)) {
        if (!is_ignore(p)) {
            ret.push_back(p);
        }
    }
    return ret;
}
consteval auto make_spec_for_ctor(std::meta::info ctor) -> std::vector<std::meta::info>
{
    using namespace std::meta;
    std::vector<info> members_spec { };
    // auto ctx = access_context::current();
    for (auto param : std::define_static_array(parameters_of(ctor))) {
        if (!is_ignore(param)) {
            if (type_of(param) == type_of(^^std::string)) {
                members_spec.push_back(
                    data_member_spec(^^char[128], { .name = identifier_of(param) }));
            }
        }
    }
    return members_spec;
}

constexpr info str_buf_t_rfl = dealias(^^std::array<char, 128>);
using str_buf_t = typename[:str_buf_t_rfl:];

consteval auto make_spec_for_param(info param) -> auto
{
    info s;
    if (type_of(param) == dealias(^^std::string)) {
        s = data_member_spec(str_buf_t_rfl,
            { .name = identifier_of(param) });
    } else {
        s = data_member_spec(type_of(param),
            { .name = identifier_of(param) });
    }
    return s;
}

template <typename Component>
consteval auto define_storage_for_component(info storage) -> void
{
    auto ctor = get_build_ctor(^^Component);
    std::vector<info> params = parameters_of(ctor);
    std::vector<info> specs { };
    for (auto p : params) {
        if (!is_ignore(p))
            specs.push_back(make_spec_for_param(p));
    }
    define_aggregate(storage, specs);
}
template <typename T>
struct Params {
    struct storage;
    consteval
    {
        define_storage_for_component<T>(^^storage);
    }
    storage data { };
};

// consteval auto unsupported_param_type_error(info comp, info param) -> void
// {
//     static_assert(false,
//         std::format(
//             "Unsupported constructor parameter '{}' type '{}' for component '{}'",
//             display_string_of(type_of(param)),
//             display_string_of(param),
//             display_string_of(comp)));
// }
#endif

inline std::unordered_map<std::string, builder_fn>
make_component_builders()
{
    auto ret = std::unordered_map<std::string, builder_fn> { };
#ifndef CLANGD_SKIP
    using namespace std::views;
    using namespace std::ranges::views;
    constexpr auto ctx = access_context::current();
    template for (constexpr auto c : std::define_static_array(get_components()))
    {
        static constexpr auto iden = std::define_static_string(identifier_of(c));
        ret[std::string(iden)] = [](game::GraphScene* s) {
            using storage_t = typename Params<typename[:c:]>::storage;
            Params<typename[:c:]> params { };
            auto& storage = params.data;
            static bool close = false;
            ImGui::Begin(iden, &close);
            template for (constexpr auto mem :
                std::define_static_array(
                    nonstatic_data_members_of(dealias(^^storage_t), ctx)))
            {
                static constexpr auto mem_iden = std::define_static_string(
                    identifier_of(mem));
                // std::cout << display_string_of(type_of(mem)) << std::endl;
                if constexpr (type_of(mem) == str_buf_t_rfl) {
                    ImGui::InputText(mem_iden,
                        storage.[:mem:]
                            .data(),
                        storage.
                                [:mem:]
                            .size());
                } else if constexpr (
                    std::is_integral_v<typename[:type_of(mem):]>) {
                    ImGui::InputInt(mem_iden,
                        &storage.[:mem:]);
                } else if constexpr (type_of(mem) == ^^float) {
                    ImGui::InputFloat(mem_iden,
                        &storage.[:mem:]);
                } else if constexpr (type_of(mem) == ^^double) {
                    ImGui::InputDouble(mem_iden,
                        &storage.[:mem:]);
                } else {
                    static_assert(false, "Unsupported constructor parameter type");
                }
                if (ImGui::Button("Create")) {
                    if (constexpr std::derived_from<
                            components::OComponent, typename[:c:]>) {
                        s->create_component<typename[:c:]>(

                                );
                    } else {
                        s->create_connection<typename[:c:]>(

                                );

                    }
                }
            }
            ImGui::End();
        };
    }
#endif
    return ret;
}
}
