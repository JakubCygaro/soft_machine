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
