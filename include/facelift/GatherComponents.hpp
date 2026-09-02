#pragma once
#include "components/Button.hpp"
#include "components/Memory.hpp"
#include "components/Display.hpp"
#include "components/Cpu.hpp"
// #include "components/Passthrough.hpp"
#include "components/Repeater.hpp"
#include "facelift/Fl.hpp"
#include "game/Scene.hpp"
#include "imgui.h"
#include <array>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#ifndef CLANGD_SKIP
#include <meta>
#endif

namespace facelift {

using builder_fn = std::function<bool(game::GraphScene*)>;

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

constexpr info str_buf_t_rfl = dealias(^^fl::string_param_t);
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
    using storage_t = storage;
    static Params default_init_members()
    {
        Params self = { };
        constexpr auto ctx = access_context::current();
        static constexpr auto dms = std::define_static_array(nonstatic_data_members_of(
            dealias(^^storage), ctx));
        template for (constexpr auto m : dms)
        {
            self.data.[:m:] = { };
        }
        return self;
    }
};

template <class T, class Storage, std::size_t... Is>
constexpr void make_from_params(
    Storage&& storage,
    game::GraphScene* s,
    std::index_sequence<Is...>)
{
    constexpr auto ctx = access_context::current();
    constexpr auto dms = std::define_static_array(nonstatic_data_members_of(
        dealias(^^Storage), ctx));
    if constexpr (std::derived_from<T,
                      components::OComponent>) {
        s->create_component<T>(
            ((std::forward<Storage>(storage).[:dms[Is]:]))...);
    }
    if constexpr (std::derived_from<T,
                      components::OConnection>) {
        s->create_connection<T>(
            ((std::forward<Storage>(storage)).[:dms[Is]:])...);
    }
};
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
        ret[std::string(iden)] = [](game::GraphScene* s) -> bool {
            using storage_t = typename Params<typename[:c:]>::storage_t;
            static Params<typename[:c:]> params = Params<
                typename[:c:]>::default_init_members();
            auto& storage = params.data;
            bool open = true;
            if (ImGui::Begin(iden, &open)) {

                template for (constexpr auto mem :
                    std::define_static_array(
                        nonstatic_data_members_of(dealias(^^storage_t), ctx)))
                {
                    static constexpr auto mem_iden = std::define_static_string(
                        identifier_of(mem));
                    // std::cout << display_string_of(dealias(^^storage_t)) << std::endl;
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
                }
                template for (constexpr auto mem :
                    std::define_static_array(
                        nonstatic_data_members_of(dealias(^^storage_t), ctx)))
                {
                    constexpr auto dms = std::define_static_array(nonstatic_data_members_of(
                        dealias(^^storage_t), ctx));
                    constexpr auto n = dms.size();

                    if (ImGui::Button("Create")) {
                        auto st = storage_t(storage);
                        make_from_params<typename[:dealias(c):], storage_t>(
                            std::move(st), s, std::make_index_sequence<n> { });
                    }
                    // BECAUSE FUCK YOU
                    break;
                }
            }
            ImGui::End();
            return !open;
        };
    }
#endif
    return ret;
}
}
