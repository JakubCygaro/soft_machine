#include "components/Cpu.hpp"
#include "game/Xml.hpp"
#include "game/XmlMarshalling.hpp"
#include <concepts>
#include <cstring>
#include <format>
#include <stdexcept>
#include <sys/types.h>
namespace components {
namespace {
}

Result<std::runtime_error, CPU::Instruction*>
CPU::instruction_from_xml(pugi::xml_node& inode)
{
    auto name = inode.name();
    auto ret = Result<std::runtime_error, CPU::Instruction*>::err(
        std::runtime_error(std::format("unknown instruction node type '{}'",
            name)));

    const auto load =
        []<typename Inst>(pugi::xml_node& node)
        -> Result<std::runtime_error, Inst*> {
        if (auto unm = game::xml::unmarshall_node<Inst>(node); unm.isok()) {
            auto i = new Inst;
            *i = unm.unwrap();
            return { i };
        } else {
            return { unm.unwrap_err() };
        }
    };

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
        ret = map(load.operator()<InstWait>(inode));
    } else if (!std::strcmp(name, "load")) {
        ret = map(load.operator()<InstLoad>(inode));
    } else if (!std::strcmp(name, "add")) {
        ret = map(load.operator()<InstAdd>(inode));
    } else if (!std::strcmp(name, "sub")) {
        ret = map(load.operator()<InstSub>(inode));
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
