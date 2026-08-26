#include "components/Cpu.hpp"
#include "game/Xml.hpp"
#include "game/XmlMarshalling.hpp"
#include "game/resources/Resources.hpp"
#include <algorithm>
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
    const auto center = get_center();
    ::DrawRectangleRounded(
        this->m_bounds,
        0.2,
        10,
        BODY_COLOR);
    auto code_d = ::Vector2 {
        .x = m_max_inst_dims.x,
        .y = m_max_inst_dims.y * m_inst_draw.size(),
    };
    auto code_p = ::Vector2 {
        .x = center.x - (code_d.x / 2.0f),
        .y = center.y - (code_d.y / 2.0f),
    };
    auto pc = 0;
    for (const auto& inst : this->m_inst_draw) {
        ::DrawRectangleRec(
            ::Rectangle {
                .x = code_p.x,
                .y = code_p.y,
                .width = m_max_inst_dims.x,
                .height = m_max_inst_dims.y,
            },
            ::WHITE);
        const auto color = (pc == m_pc) ? ::RED : CPU::INST_COLOR;
        ::DrawTextEx(
            game::resources::get_node_font(),
            inst.rep.c_str(),
            code_p,
            game::resources::default_node_font_size(),
            game::resources::default_font_spacing(),
            color);
        code_p.y += (inst.dims.y);
        pc++;
    }
    using namespace game::resources;
    ::DrawTextEx(
        get_node_font(),
        get_name().c_str(),
        { center.x - m_name_sz.x / 2, m_bounds.y },
        default_node_font_size(),
        default_font_spacing(),
        CPU::INST_COLOR);
}
void CPU::update()
{
}
void CPU::setup(CPU& self)
{
    self.m_inst_draw.clear();
    self.m_max_inst_dims = { };
    for (const auto& inst : self.m_code) {
        const auto rep = inst->to_render_string();
        const auto dims = ::MeasureTextEx(
            game::resources::get_node_font(),
            rep.c_str(),
            game::resources::default_node_font_size(),
            game::resources::default_font_spacing());
        self.m_max_inst_dims = {
            .x = std::max(self.m_max_inst_dims.x, dims.x),
            .y = std::max(self.m_max_inst_dims.y, dims.y),
        };
        self.m_inst_draw.push_back({
            .rep = rep,
            .dims = dims,
        });
    }
    self.m_bounds = ::Rectangle {
        .x = self.m_bounds.x,
        .y = self.m_bounds.y,
        .width = self.m_max_inst_dims.x * 1.4f,
        .height = (self.m_max_inst_dims.y * self.m_inst_draw.size()) * 1.8f,
    };
    using namespace game::resources;
    self.m_name_sz = ::MeasureTextEx(
        get_node_font(),
        self.get_name().c_str(),
        default_node_font_size(),
        default_font_spacing());
}

machine::actor::Actor
CPU::poll(machine::Mctx ctx)
{
    while (1) {
        co_await ctx.pause();
    }
}
}
