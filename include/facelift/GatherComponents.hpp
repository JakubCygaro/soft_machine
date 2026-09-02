#pragma once
#include "components/Memory.hpp"
#include "facelift/Fl.hpp"
#include "game/Scene.hpp"
#include "machine/MachineGraph.hpp"
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
    using namespace std::meta;
    auto ctx = access_context::current();
    return std::define_static_array(members_of(^^::components, ctx));
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
#endif

inline std::unordered_map<std::string, builder_fn>
make_component_builders()
{
    auto ret = std::unordered_map<std::string, builder_fn> { };
#ifndef CLANGD_SKIP
    using namespace std::meta;
    using namespace std::views;
    using namespace std::ranges::views;
    constexpr auto ctx = access_context::current();
    template for (constexpr auto c : get_components())
    {
        if constexpr (is_component(c)) {
            constexpr auto iden = identifier_of(c);
            std::cout << identifier_of(c) << std::endl;
            constexpr auto build_ctor = get_build_ctor(c);
            if constexpr (!has_build_ctor(c))
                continue;

            struct Params;
            consteval
            {
                std::meta::define_aggregate(^^Params,
                    make_spec_for_ctor(get_build_ctor(c)));
            }

            ret[std::string(iden)] = [](game::GraphScene* s) {
                // for (auto param : std::define_static_array(parameters_of(*build_ctor))) {
                // if constexpr (!is_ignore(param)
                //         && std::derived_from_v<components::OComponent, typename [:type_of(c):]>) {
                //     s->create_component()
                // }
                // }
            };
        }
    }
#endif
    return ret;
}
}
