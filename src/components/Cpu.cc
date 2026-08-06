#include "components/Cpu.hpp"
#include "game/Xml.hpp"
#include <cctype>
#include <concepts>
#include <cstring>
#include <format>
#include <stdexcept>
namespace components {
namespace {
    using err_t = std::runtime_error;
    Result<std::runtime_error, int>
    parse_reg_string(const std::string& str)
    {
        if (str.size() != 2
            || (str[0] != 'R' && str[0] != 'r')
            || !std::isdigit(str[1])
            || (str[1] - '0') >= static_cast<int>(CPU::REG_COUNT)) {
            return { err_t("not a valid register") };
        }
        return {
            str[1] - '0'
        };
    }
    using src_dst_t = std::pair<int, int>;
    Result<std::runtime_error, src_dst_t>
    get_src_dst(const pugi::xml_node& node)
    {
        auto src = game::attribute_as_string(node, "src");
        if (src.iserr())
            return { src.unwrap_err() };
        auto src_reg = parse_reg_string(src.unwrap());
        if (src_reg.iserr())
            return { src_reg.unwrap_err() };
        auto dst = game::attribute_as_string(node, "dst");
        if (dst.iserr())
            return { dst.unwrap_err() };
        auto dst_reg = parse_reg_string(dst.unwrap());
        if (dst_reg.iserr())
            return { dst_reg.unwrap_err() };
        return { std::make_pair(src_reg.unwrap(), dst_reg.unwrap()) };
    }

    Result<std::runtime_error, CPU::InstWait*>
    wait_from_xml(pugi::xml_node& wait)
    {
        auto on = game::attribute_as_string(wait, "on");
        if (on.iserr())
            return { on.unwrap_err() };
        auto r = game::attribute_as_string(wait, "into");
        if (r.iserr())
            return { r.unwrap_err() };
        auto reg = parse_reg_string(r.unwrap());
        if (reg.iserr())
            return { reg.unwrap_err() };
        auto i = new CPU::InstWait;
        i->on = on.unwrap();
        i->into = reg.unwrap();
        return { i };
    }
    Result<std::runtime_error, CPU::InstLoad*>
    load_from_xml(pugi::xml_node& load)
    {
        auto from = game::attribute_as_string(load, "from");
        if (from.iserr())
            return { from.unwrap_err() };
        auto at = game::attribute_as_string(load, "at");
        if (at.iserr())
            return { at.unwrap_err() };
        auto into = game::attribute_as_string(load, "into");
        if (into.iserr())
            return { into.unwrap_err() };
        auto at_idx = parse_reg_string(at.unwrap());
        if (at_idx.iserr())
            return { at_idx.unwrap_err() };
        auto into_reg = parse_reg_string(at.unwrap());
        if (at_idx.iserr())
            return { at_idx.unwrap_err() };
        auto i = new CPU::InstLoad;
        i->from = from.unwrap();
        i->at = at_idx.unwrap();
        i->into = into_reg.unwrap();
        return { i };
    }
    Result<std::runtime_error, CPU::InstAdd*>
    add_from_xml(pugi::xml_node& add)
    {
        auto params = get_src_dst(add);
        if (params.iserr())
            return { params.unwrap_err() };
        auto [s, d] = params.unwrap();
        auto i = new CPU::InstAdd;
        i->src = s;
        i->dst = d;
        return { i };
    }
}

Result<std::runtime_error, CPU::Instruction*>
CPU::instruction_from_xml(pugi::xml_node& inode)
{
    auto name = inode.name();
    auto ret = Result<std::runtime_error, CPU::Instruction*>::err(
        std::runtime_error(std::format("unknown instruction node type '{}'",
            name)));

    const auto map =
        []<std::derived_from<CPU::Instruction> T>(
            Result<std::runtime_error, T*> r)
        -> Result<std::runtime_error, CPU::Instruction*> {
        if (r.isok())
            return { static_cast<Instruction*>(r.unwrap()) };
        else
            return { r.unwrap_err() };
    };

    if (!std::strcmp(name, "wait")) {
        ret = map(wait_from_xml(inode));
    } else if (!std::strcmp(name, "load")) {
        ret = map(load_from_xml(inode));
    } else if (!std::strcmp(name, "add")) {
        ret = map(add_from_xml(inode));
    }

    return ret;
}

void CPU::draw()
{
}
void CPU::update()
{
}

machine::actor::Actor
CPU::poll(machine::Mctx ctx)
{
    while (1) {
        co_await ctx.pause();
    }
}
}
