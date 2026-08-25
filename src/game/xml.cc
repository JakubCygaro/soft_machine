#include "game/Xml.hpp"
#include "common/Result.hpp"
#include "common/String.hpp"
#include "components/Cpu.hpp"
#include "components/Memory.hpp"
#include "components/Passthrough.hpp"
#include "game/XmlMarshalling.hpp"
#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstring>
#include <format>
#include <iostream>
#include <memory>
#include <optional>
#include <raylib.h>
#include <string>
#include <system_error>
#include <type_traits>

namespace {
using err_t = std::runtime_error;
using ret_t = unit_result_t<err_t>;

std::optional<::Vector2>
get_position_node(pugi::xml_node& n)
{
    auto p_n = n.child("position");
    if (!p_n) {
        return std::nullopt;
    }
    if (auto v = game::xml::unmarshall_attributes<::Vector2>(p_n); v.isok()) {
        return { v.unwrap() };
    }
    // auto ret = ::Vector2 { };
    // if (auto x = p_n.attribute("x")) {
    //     ret.x = x.as_int();
    // }
    // if (auto y = p_n.attribute("y")) {
    //     ret.y = y.as_int();
    // }
    return std::nullopt;
}

using name_from_to_t = std::tuple<std::string, std::string, std::string>;
Result<err_t, name_from_to_t>
get_name_from_and_to(pugi::xml_node& n)
{
    using ret = Result<err_t, name_from_to_t>;
    auto name_a = n.attribute("name");
    if (!name_a)
        return { err_t("No 'name' defined for connector") };
    auto name = name_a.as_string();
    if (!name)
        return ret::err(err_t("Empty 'name' field"));
    auto from_a = n.attribute("from");
    if (!from_a)
        return ret::err(err_t("No 'from' defined for connector"));
    auto from = from_a.as_string();
    if (!from)
        return ret::err(err_t("Empty 'from' field"));
    auto to_a = n.attribute("to");
    if (!to_a)
        return ret::err(err_t("No 'to' defined for connector"));
    auto to = to_a.as_string();
    if (!to)
        return ret::err(err_t("Empty 'to' field"));
    return ret::ok(name_from_to_t(name, from, to));
}

template <typename T>
struct build {
    Result<err_t, T*>
    operator()(machine::MachineGraph& mg, pugi::xml_node& n) const noexcept;
};

template <>
struct build<components::Passthrough> {
    Result<err_t, components::Passthrough*>
    operator()(machine::MachineGraph& mg, pugi::xml_node& n) const noexcept
    {
        auto a = get_name_from_and_to(n);
        if (a.iserr())
            return { a.unwrap_err() };
        auto [name, f, t] = std::get<name_from_to_t>(a);
        auto p = mg.create_connection<components::Passthrough>(
            name,
            f,
            t);
        return Result<err_t, components::Passthrough*>::ok(p);
    }
};

template <>
struct build<components::CPU> {
    Result<err_t, std::vector<components::CPU::Instruction*>>
    parse_code(pugi::xml_node& n) const noexcept
    {
        std::vector<components::CPU::Instruction*> ret { };
        for (auto ch : n.children()) {
            auto res = components::CPU::instruction_from_xml(ch);
            if (res.iserr())
                return { res.unwrap_err() };
            ret.push_back(res.unwrap());
        }
        return { ret };
    }
    Result<err_t, components::CPU*>
    operator()(machine::MachineGraph& mg, pugi::xml_node& n) const noexcept
    {
        struct cpu_node {
            struct cpu_attrs {
                std::string name;
            };
            game::xml::Attribute<cpu_attrs> attrs;
            struct code_node {
                std::vector<components::CPU::Instruction*> insts { };
                auto unmarshall_self(const pugi::xml_node& n)
                    -> Result<std::runtime_error, Unit>
                {
                    for (auto ch : n.children()) {
                        auto res = components::CPU::instruction_from_xml(ch);
                        if (res.iserr())
                            return { res.unwrap_err() };
                        insts.push_back(res.unwrap());
                    }
                    return { unit() };
                }
            } code;
        };
        cpu_node cpu;
        if (auto unm = game::xml::unmarshall_node<cpu_node>(n); unm.iserr()) {
            return { unm.unwrap_err() };
        } else {
            cpu = unm.unwrap();
        }
        components::CPU::code_t inst { };
        std::for_each(cpu.code.insts.begin(), cpu.code.insts.end(), [&](auto i) {
            // because why the fuck not?
            using ptr_t = typename components::CPU::code_t::value_type::pointer;
            inst.push_back(
                std::unique_ptr<std::remove_pointer_t<ptr_t>>(i));
        });
        auto p = mg.create_component<components::CPU>(
                cpu.attrs->name,
                std::move(inst));
        return { p };
    }
};

template <>
struct build<components::Memory> {
    struct mem_node {
        struct memset_node {
            components::Memory::mem_t mem;
            static Result<std::runtime_error, memset_node>
            parse_xml(pugi::xml_text txt) noexcept
            {
                const auto parse_int =
                    [](std::string& slice) -> Result<std::runtime_error, int> {
                    int out;
                    auto trimmed = common::trim(slice);
                    auto res = std::from_chars(
                        trimmed.data(),
                        trimmed.data() + trimmed.size(),
                        out,
                        10);
                    if (res.ec == std::errc::invalid_argument) {
                        return { std::runtime_error(
                            std::format("failed to parse '{}' as integer", slice)) };
                    }
                    return { out };
                };
                auto s = txt.as_string();
                if (!s)
                    return { std::runtime_error("empty memset node") };
                components::Memory::mem_t ret { };
                auto str = common::trim(std::string(s));
                str.erase(std::remove_if(
                              str.begin(),
                              str.end(),
                              [](auto c) {
                                  return std::isspace(c) && c != ' ';
                              }),
                    str.end());
                size_t pos = str.find(' ');
                size_t init_pos = 0;

                while (pos != std::string::npos) {
                    auto i = common::trim(str.substr(init_pos, pos - init_pos));
                    if (!i.empty()) {
                        auto res = parse_int(i);
                        if (res.iserr())
                            return { res.unwrap_err() };
                        ret.push_back(std::get<int>(res));
                    }
                    init_pos = pos + 1;
                    pos = str.find(' ', init_pos);
                }
                auto i = common::trim(str.substr(
                    init_pos,
                    std::min(
                        pos,
                        str.size())
                        - init_pos + 1));
                if (!i.empty()) {
                    auto res = parse_int(i);
                    if (res.iserr())
                        return { res.unwrap_err() };
                    ret.push_back(std::get<int>(res));
                }
                memset_node m { };
                m.mem = std::move(ret);
                return { m };
            }
        };
        memset_node memset;
        struct mem_attr {
            std::string name;
            components::Memory::Layout layout;
        };
        game::xml::Attribute<mem_attr> attrs;
    };
    Result<err_t, components::Memory*>
    operator()(machine::MachineGraph& mg, pugi::xml_node& n) const noexcept
    {
        mem_node mem_n;
        if (auto unm = game::xml::unmarshall_node<mem_node>(n); unm.iserr()) {
            return { unm.unwrap_err() };
        } else {
            mem_n = unm.unwrap();
        }
        auto p = mg.create_component<components::Memory>(
            mem_n.attrs->name,
            std::move(mem_n.memset.mem));
        p->set_layout(mem_n.attrs->layout);

        return { p };
    }
};
ret_t build_from_xml_node(machine::MachineGraph& mg, pugi::xml_node& n)
{
    auto name = n.name();
    if (std::strcmp("passthrough", name) == 0) {
        auto res = build<components::Passthrough>()(mg, n);
        if (res.iserr())
            return { res.unwrap_err() };
    }
    components::OComponent* as_comp = nullptr;
    if (std::strcmp("memory", name) == 0) {
        auto res = build<components::Memory>()(mg, n);
        if (res.iserr())
            return { res.unwrap_err() };
        as_comp = res.unwrap();
    }
    if (std::strcmp("cpu", name) == 0) {
        auto res = build<components::CPU>()(mg, n);
        if (res.iserr())
            return { res.unwrap_err() };
        as_comp = res.unwrap();
    }
    if (as_comp) {
        if (auto pos = get_position_node(n)) {
            as_comp->set_pos(*pos);
        }
    }
    return { unit() };
}
}
namespace game {
Result<std::runtime_error, Unit>
populate_machine_from_xml(machine::MachineGraph& mg, const std::string& xml)
{
    pugi::xml_document doc;
    pugi::xml_parse_result res = doc.load_string(xml.c_str());
    if (!res)
        return { std::runtime_error(res.description()) };
    auto graph_node = doc.child("graph");
    if (!graph_node)
        return { std::runtime_error(
            "graph node is not present in the xml schema") };
    for (auto elem : graph_node.children()) {
        auto res = build_from_xml_node(mg, elem);
        if (res.iserr()) {
            return { res.unwrap_err() };
        }
    }
    return { unit() };
}
Result<std::runtime_error, std::string>
attribute_as_string(
    const pugi::xml_node& wait,
    const std::string& attribute)
{
    using err_t = std::runtime_error;
    auto on_a = wait.attribute(attribute);
    if (!on_a)
        return { err_t(std::format("'{}' attribute missing", attribute)) };
    auto on = on_a.as_string();
    if (!on)
        return { err_t("'{}' attribute is not a string") };
    return { std::string(on) };
}
}
