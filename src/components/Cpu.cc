#include "components/Cpu.hpp"
#include "components/Memory.hpp"
#include "game/Xml.hpp"
#include "game/XmlMarshalling.hpp"
#include "game/resources/Resources.hpp"
#include <algorithm>
#include <concepts>
#include <cstring>
#include <format>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string>
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
    } else if (!std::strcmp(name, "send")) {
        ret = map(load.operator()<InstSend>(inode));
    } else if (!std::strcmp(name, "if")) {
        ret = map(load.operator()<InstIf>(inode));
    } else if (!std::strcmp(name, "movi")) {
        ret = map(load.operator()<InstMovi>(inode));
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
        .y = m_inst_dims_y_acc,
    };
    auto code_p = ::Vector2 {
        .x = center.x - (code_d.x / 2.0f),
        .y = center.y - (code_d.y / 2.0f),
    };
    using namespace std::ranges::views;
    using namespace std::views;
    using namespace game::resources;
    for (const auto& [inst, pc] : zip(this->m_inst_draw, iota(0))) {
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
    }
    auto reg_p = ::Vector2 {
        .x = center.x - (code_d.x / 2.0f),
        .y = center.y - (code_d.y / 2.0f) - (m_max_reg_dims.y * 1.1f),
    };
    for (const auto& [r, ridx] : zip(this->m_reg_draw_data, iota(0))) {
        auto bounds = ::Rectangle {
            .x = reg_p.x,
            .y = reg_p.y,
            .width = m_max_reg_dims.x,
            .height = m_max_reg_dims.y,
        };
        ::DrawRectangleRec(
            bounds,
            ::WHITE);
        ::DrawRectangleLinesEx(
            bounds,
            1.0f,
            ::BLACK);
        auto cell_center = ::Vector2Add(
            { bounds.x, bounds.y },
            ::Vector2Scale(
                m_max_reg_dims,
                0.5));
        auto v_pos = ::Vector2Subtract(
            cell_center,
            ::Vector2Scale(
                r.dims,
                0.5));
        const auto c = (r.marked) ? CPU::MARKED_COLOR : CPU::INST_COLOR;
        ::DrawTextEx(
            get_node_font(),
            r.rep.c_str(),
            v_pos,
            CPU::REG_FONT_SIZE,
            default_font_spacing(),
            c);
        reg_p.x += (m_max_reg_dims.x);
    }
    auto pc_bounds = ::Rectangle {
        .x = center.x - (code_d.x / 2.0f),
        .y = center.y - (code_d.y / 2.0f)
            - (m_max_reg_dims.y * 1.1f)
            - (std::get<::Vector2>(m_pc_draw_data).y * 1.1f),
        .width = std::get<::Vector2>(m_pc_draw_data).x * 1.8f,
        .height = std::get<::Vector2>(m_pc_draw_data).y,
    };
    ::DrawRectangleRec(
        pc_bounds,
        ::WHITE);
    ::DrawRectangleLinesEx(
        pc_bounds,
        1.0f,
        ::BLACK);
    auto pc_cell_center = ::Vector2Add(
        { pc_bounds.x, pc_bounds.y },
        ::Vector2Scale(
            { pc_bounds.width, pc_bounds.height },
            0.5));
    auto pc_pos = ::Vector2Subtract(
        pc_cell_center,
        ::Vector2Scale(
            std::get<::Vector2>(m_pc_draw_data),
            0.5));
    ::DrawTextEx(
        get_node_font(),
        std::get<std::string>(m_pc_draw_data).c_str(),
        pc_pos,
        CPU::REG_FONT_SIZE,
        default_font_spacing(),
        CPU::INST_COLOR);

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
    setup_regs(*this);
    setup_bounds(*this);
}
void CPU::unmark_all(void)
{
    std::ranges::for_each(m_reg_draw_data,
        [](auto& rd) { rd.marked = false; });
}
int& CPU::reg(const Register& r)
{
    return m_regs[r.idx];
}
void CPU::setup(CPU& self)
{
    self.m_inst_draw.clear();
    self.m_max_inst_dims = { };
    self.m_inst_dims_y_acc = 0;
    for (const auto& inst : self.m_code) {
        const auto rep = inst->to_render_string();
        const auto dims = ::MeasureTextEx(
            game::resources::get_node_font(),
            rep.c_str(),
            game::resources::default_node_font_size(),
            game::resources::default_font_spacing());
        self.m_max_inst_dims = {
            .x = std::max(self.m_max_inst_dims.x, dims.x * 1.2f),
            .y = std::max(self.m_max_inst_dims.y, dims.y * 1.0f),
        };
        self.m_inst_draw.push_back({
            .rep = rep,
            .dims = dims,
        });
        self.m_inst_dims_y_acc += dims.y;
    }
    self.m_inst_dims_y_acc *= 1.2f;
    setup_regs(self);
    setup_pc(self);
    using namespace game::resources;
    self.m_name_sz = ::MeasureTextEx(
        get_node_font(),
        self.get_name().c_str(),
        default_node_font_size(),
        default_font_spacing());
    setup_bounds(self);
}
void CPU::setup_pc(CPU& self)
{
    using namespace game::resources;
    std::get<std::string>(self.m_pc_draw_data) = std::to_string(self.m_pc);
    std::get<::Vector2>(self.m_pc_draw_data) = ::MeasureTextEx(
        get_node_font(),
        std::get<std::string>(self.m_pc_draw_data).c_str(),
        CPU::REG_FONT_SIZE,
        default_font_spacing());
}
void CPU::setup_regs(CPU& self)
{
    using namespace std::ranges::views;
    using namespace std::views;
    for (const auto& [reg, i] : zip(self.m_regs, iota(0))) {
        const auto rep = std::to_string(reg);
        const auto dims = ::MeasureTextEx(
            game::resources::get_node_font(),
            rep.c_str(),
            CPU::REG_FONT_SIZE,
            game::resources::default_font_spacing());
        self.m_max_reg_dims = {
            .x = std::max(self.m_max_reg_dims.x, dims.x * 1.5f),
            .y = std::max(self.m_max_reg_dims.y, dims.y * 1.5f),
        };
        self.m_reg_draw_data[i] = {
            .rep = rep,
            .dims = dims,
            .marked = false,
        };
    }
}
void CPU::setup_bounds(CPU& self)
{
    self.m_bounds = ::Rectangle {
        .x = self.m_bounds.x,
        .y = self.m_bounds.y,
        .width = (self.m_max_inst_dims.x
                     + self.m_max_reg_dims.x * self.m_regs.size())
            * 1.0f,
        .height = (self.m_inst_dims_y_acc
                      + self.m_max_reg_dims.y
                      + self.m_name_sz.y
                      + std::get<::Vector2>(self.m_pc_draw_data).y)
            * 1.2f,
    };
}
machine::actor::Actor
CPU::poll(machine::Mctx ctx)
{
    while (1) {
        if (m_code.empty()) {
            co_await ctx.pause();
            continue;
        }
        const auto* inst = m_code[m_pc].get();

        if (auto i = dynamic_cast<const InstWait*>(inst)) {
            while (1) {
                auto [s, m] = co_await ctx.recv();
                if (s != i->attrs->on)
                    continue;
                if (auto as_int = std::any_cast<int>(&m); !as_int)
                    continue;
                else {
                    m_regs[i->attrs->into.idx] = *as_int;
                    m_reg_draw_data[i->attrs->into.idx].marked = true;
                    break;
                }
            }
        }
        if (auto i = dynamic_cast<const InstLoad*>(inst)) {
            while (1) {
                auto s = co_await ctx.send(
                    i->attrs->from,
                    components::mem::MemoryRead(i->attrs->at));
                if (s.iserr())
                    continue;
                break;
            }
            while (1) {
                auto [s, m] = co_await ctx.recv();
                if (s != i->attrs->from)
                    continue;
                if (auto mem_fail = std::any_cast<mem::MemoryFail>(&m)) {
                    // TODO: memory read failure handling
                    (void)mem_fail;
                }
                if (auto mem_succ = std::any_cast<mem::MemoryResponse>(&m)) {
                    m_regs[i->attrs->into.idx] = mem_succ->val;
                    m_reg_draw_data[i->attrs->into.idx].marked = true;
                    break;
                }
            }
        }
        if (auto i = dynamic_cast<const InstIf*>(inst)) {
            const auto& a = reg(i->attrs->a);
            const auto& b = reg(i->attrs->b);
            std::cout << "a: " << a << " b: " << b << std::endl;
            bool res = false;
            switch (i->attrs->op) {
            case InstIf::Op::L:
                res = a < b;
                break;
            case InstIf::Op::LE:
                res = a <= b;
                break;
            case InstIf::Op::E:
                res = a == b;
                break;
            case InstIf::Op::NE:
                res = a != b;
                break;
            case InstIf::Op::G:
                res = a > b;
                break;
            case InstIf::Op::GE:
                res = a >= b;
                break;
            }
            if (res)
                m_pc = i->attrs->jmpto - 1;
        }
        if (auto i = dynamic_cast<const InstMovi*>(inst)) {
            reg(i->attrs->into) = i->attrs->ival;
        }
        if (auto i = dynamic_cast<const InstSend*>(inst)) {
            co_await ctx.send(i->attrs->to, reg(i->attrs->from));
        }
        if (auto i = dynamic_cast<const InstAdd*>(inst)) {
            reg(i->attr->dst) += reg(i->attr->src);
        }
        if (auto i = dynamic_cast<const InstSub*>(inst)) {
            reg(i->attr->dst) -= reg(i->attr->src);
        }

        m_pc++;
        if (m_pc >= static_cast<int>(m_code.size()))
            m_pc = 0;
        setup_pc(*this);
        co_await ctx.pause();
        unmark_all();
    }
}
}
