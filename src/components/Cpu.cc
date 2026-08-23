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
    using err_t = std::runtime_error;
    struct src_dst_attr {
        CPU::Register src, dst;
    };
    Result<std::runtime_error, src_dst_attr>
    get_src_dst(const pugi::xml_node& node)
    {
        return game::xml::unmarshall_attributes<src_dst_attr>(node);
    }

    Result<std::runtime_error, CPU::InstWait*>
    wait_from_xml(pugi::xml_node& wait)
    {
        struct wait_attr {
            std::string on;
            CPU::Register into;
        };
        if (auto unm = game::xml::unmarshall_attributes<wait_attr>(wait); unm.isok()) {
            auto attrs = unm.unwrap();
            auto i = new CPU::InstWait;
            i->on = attrs.on;
            i->into = attrs.into.idx;
            return { i };
        } else {
            return { unm.unwrap_err() };
        }
    }
    Result<std::runtime_error, CPU::InstLoad*>
    load_from_xml(pugi::xml_node& load)
    {
        struct load_attr {
            std::string from;
            int at;
            CPU::Register into;
        };
        auto l = game::xml::unmarshall_attributes<load_attr>(load);
        if (l.iserr())
            return { l.unwrap_err() };
        auto attrs = l.unwrap();
        auto i = new CPU::InstLoad;
        i->from = attrs.from;
        i->at = attrs.at;
        i->into = attrs.into.idx;
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
        i->src = s.idx;
        i->dst = d.idx;
        return { i };
    }
    Result<std::runtime_error, CPU::InstSub*>
    sub_from_xml(pugi::xml_node& sub)
    {
        if (auto unm = game::xml::unmarshall_node<CPU::InstSub>(sub); unm.isok()) {
            auto i = new CPU::InstSub;
            *i = unm.unwrap();
            return { i };
        } else {
            return { unm.unwrap_err() };
        }
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
    } else if (!std::strcmp(name, "sub")) {
        ret = map(sub_from_xml(inode));
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
