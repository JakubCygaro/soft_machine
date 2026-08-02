#include "game/Xml.hpp"
#include "common/String.hpp"
#include "components/Cpu.hpp"
#include "components/Memory.hpp"
#include "components/Passthrough.hpp"
#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstring>
#include <memory>
#include <optional>
#include <raylib.h>
#include <string>
#include <system_error>

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
    auto ret = ::Vector2 { };
    if (auto x = p_n.attribute("x")) {
        ret.x = x.as_int();
    }
    if (auto y = p_n.attribute("y")) {
        ret.y = y.as_int();
    }
    return ret;
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
    Result<err_t, components::CPU::code_t>
    parse_code(pugi::xml_node& n) const noexcept
    {
        components::CPU::code_t ret { };
        for (auto ch : n.children()) {
            auto res = components::CPU::instruction_from_xml(ch);
            if (res.iserr())
                return { res.unwrap_err() };
            ret.push_back(
                std::unique_ptr<components::CPU::Instruction>(res.unwrap()));
        }
        return Result<err_t, components::CPU::code_t>::ok(ret);
    }
    Result<err_t, components::CPU*>
    operator()(machine::MachineGraph& mg, pugi::xml_node& n) const noexcept
    {
        auto name_a = n.attribute("name");
        if (!name_a)
            return { err_t("No 'name' defined for connector") };
        auto name = name_a.as_string();
        if (!name)
            return { err_t("Empty 'name' field") };
        auto code_n = n.child("code");
        if (!code_n)
            return { err_t("code node not defined on cpu component") };
        auto code = parse_code(code_n);
        if (code.iserr())
            return { code.unwrap_err() };
        auto c = code.unwrap();
        auto p = mg.create_component<components::CPU>(
            name,
            std::move(c));
        return { p };
    }
};

template <>
struct build<components::Memory> {
    Result<std::runtime_error, components::Memory::mem_t>
    parse_mem(pugi::xml_text txt) const noexcept
    {
        const auto parse_int =
            [](std::string& slice) -> Result<std::runtime_error, int> {
            int out;
            auto res = std::from_chars(
                slice.data(),
                slice.data() + slice.size(),
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
        size_t pos = str.find(' ');
        size_t init_pos = 0;

        while (pos != std::string::npos) {
            auto i = str.substr(init_pos, pos - init_pos);
            auto res = parse_int(i);
            if (res.iserr())
                return { res.unwrap_err() };
            ret.push_back(std::get<int>(res));
            init_pos = pos + 1;
            pos = str.find(' ', init_pos);
        }
        auto i = str.substr(
            init_pos,
            std::min(
                pos,
                str.size())
                - init_pos + 1);

        auto res = parse_int(i);
        if (res.iserr())
            return { res.unwrap_err() };
        ret.push_back(std::get<int>(res));

        return { ret };
    }
    Result<err_t, components::Memory*>
    operator()(machine::MachineGraph& mg, pugi::xml_node& n) const noexcept
    {
        auto name_a = n.attribute("name");
        if (!name_a)
            return { err_t("No 'name' defined for connector") };
        auto name = name_a.as_string();
        if (!name)
            return { err_t("Empty 'name' field") };
        auto mem_n = n.child("memset");
        if (!mem_n)
            return { err_t("memset not defined on memory component") };
        auto mem = parse_mem(mem_n.text());
        if (mem.iserr())
            return { mem.unwrap_err() };
        auto p = mg.create_component<components::Memory>(
            name,
            std::move(
                std::get<components::Memory::mem_t>(mem)));
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
