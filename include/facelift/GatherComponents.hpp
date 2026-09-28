#pragma once
#include "components/Button.hpp"
#include "components/Cpu.hpp"
#include "components/Display.hpp"
#include "components/Memory.hpp"
// #include "components/Passthrough.hpp"
#include "components/Repeater.hpp"
#include "facelift/Fl.hpp"
#include "game/Drawable.hpp"
#include "game/Scene.hpp"
#include "imgui.h"
#include <array>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#ifndef CLANGD_SKIP
#include <meta>
#endif

namespace facelift {

struct BuilderBase {
protected:
    std::optional<std::runtime_error> last_error { };
    bool should_close { };

public:
    bool is_should_close() noexcept;
    virtual const std::string& get_element_name() = 0;
    virtual ~BuilderBase() = default;

protected:
    void set_should_close() noexcept;
};

struct RuntimeComponentBuilder : public BuilderBase {
public:
    virtual components::OComponent*
    display_builder(game::GraphScene*) = 0;
    virtual ~RuntimeComponentBuilder() = default;
};
template <std::derived_from<components::OComponent> Component>
struct ComponentBuilder : public RuntimeComponentBuilder {
    using comp_t = Component;
    virtual ~ComponentBuilder() = default;
};

struct RuntimeConnectionBuilder : public BuilderBase {
    virtual components::OConnection*
    display_builder(game::GraphScene*, const std::string&, const std::string&) = 0;
    virtual ~RuntimeConnectionBuilder() = default;
};
template <std::derived_from<components::OConnection> Connection>
struct ConnectionBuilder : public RuntimeConnectionBuilder {
    using conn_t = Connection;
    virtual ~ConnectionBuilder() = default;
    virtual const std::string& get_element_name() = 0;
};

// using comp_builder_fn = std::function<std::pair<bool,
//     std::optional<components::OComponent*>>(game::GraphScene*)>;
// using conn_builder_fn = std::function<std::pair<bool,
//     std::optional<components::OConnection*>>(
//     game::GraphScene*, const std::string&, const std::string&)>;

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

std::unordered_map<std::string, std::unique_ptr<RuntimeComponentBuilder>>
runtime_make_component_builders();
std::unordered_map<std::string, std::unique_ptr<RuntimeConnectionBuilder>>
runtime_make_connection_builders();
// std::unordered_map<std::string, comp_builder_fn>
// runtime_make_component_builders();
// std::unordered_map<std::string, conn_builder_fn>
// runtime_make_connection_builders();
}
