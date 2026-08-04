#include "components/Memory.hpp"
#include "game/resources/Resources.hpp"
#include <algorithm>
#include <any>
#include <cmath>
#include <raylib.h>
#include <raymath.h>
#include <string>
using namespace game::resources;
namespace components {
void Memory::draw()
{
    const auto center = ::Vector2Add(
        ::Vector2Scale(
            { m_bounds.width, m_bounds.height },
            0.5),
        { m_bounds.x, m_bounds.y });
    ::DrawRectangleRounded(
        this->m_bounds,
        0.2,
        10,
        BODY_COLOR);
    // ::DrawCircleV(center, 10, ::RED);
    const auto mem_pos = ::Vector2Subtract(
        center,
        ::Vector2Scale(
            m_mem_sz,
            0.5));
    ::DrawRectangleRec(
        ::Rectangle {
            .x = mem_pos.x,
            .y = mem_pos.y,
            .width = m_mem_sz.x,
            .height = m_mem_sz.y,
        },
        MEM_CELL_COLOR);

    switch (m_lay) {
    case Layout::Square: {
        auto side = static_cast<size_t>(
            std::sqrt(
                static_cast<float>(
                    m_mem_strs.size())));
        auto cell_start = ::Vector2Subtract(
            center,
            ::Vector2Scale(
                m_cell_sz,
                side / 2.0f));
        auto s = m_mem_strs.begin();
        for (auto i = 0UL; i < side; i++) {
            for (auto j = 0UL; j < side; j++) {
                if(s >= m_mem_strs.end()) break;
                auto pos = ::Vector2 {
                    .x = cell_start.x + (j * m_cell_sz.x),
                    .y = cell_start.y + (i * m_cell_sz.y),
                };
                ::DrawTextEx(
                    get_node_font(),
                    (*s++).c_str(),
                    pos,
                    default_node_font_size(),
                    default_font_spacing(),
                    DATA_COLOR);
            }
            if(s >= m_mem_strs.end()) break;
        }
    } break;
    case Layout::Horizontal: {

    } break;
    case Layout::Vertical: {

    } break;
    }
    ::DrawTextEx(
        get_node_font(),
        get_name().c_str(),
        { center.x - m_name_sz.x / 2, m_bounds.y },
        default_node_font_size(),
        default_font_spacing(),
        DATA_COLOR);
}
void Memory::update()
{
}
std::pair<::Vector2, std::vector<std::string>>
Memory::setup_mem(const mem_t& mem)
{
    std::vector<std::string> out { };
    ::Vector2 sz = { };
    const auto measure_sz = [&](const auto& s) {
        auto m = ::MeasureTextEx(
            get_node_font(),
            s.c_str(),
            default_node_font_size(),
            default_font_spacing());
        sz.x = std::max(m.x * 2.2f, sz.x);
        sz.y = std::max(m.y * 2.2f, sz.y);
    };
    std::for_each(mem.begin(), mem.end(), [&](auto integer) {
        out.push_back(std::to_string(integer));
        measure_sz(out.back());
    });
    return std::make_pair(sz, out);
}
void Memory::setup(Memory& self)
{
    auto [cell, ints] = setup_mem(self.m_mem);
    constexpr const auto def = game::resources::get_default_node_size();
    self.m_mem_sz = cell;
    self.m_cell_sz = cell;
    self.m_name_sz = ::MeasureTextEx(
        get_node_font(),
        self.get_name().c_str(),
        default_node_font_size(),
        default_font_spacing());
    switch (self.m_lay) {
    case Memory::Layout::Square: {
        const auto side = static_cast<size_t>(
            std::sqrt(
                static_cast<float>(
                    ints.size())));
        self.m_bounds.width = def.x + 40.0f + self.m_name_sz.x + cell.x * side;
        self.m_bounds.height = def.y + 40.0f + self.m_name_sz.y + cell.y * side;
    } break;
    case Memory::Layout::Vertical: {
        self.m_bounds.width = def.x + 40.0f + self.m_name_sz.x + cell.x * ints.size();
        self.m_bounds.height = def.y + 40.0f + self.m_name_sz.y + cell.y;
    } break;
    case Memory::Layout::Horizontal: {
        self.m_bounds.width = def.x + 40.0f + self.m_name_sz.x + cell.x;
        self.m_bounds.height = def.y + 40.0f + self.m_name_sz.y + cell.y * ints.size();
    } break;
    }
    self.m_mem_strs = ints;
}
machine::actor::Actor Memory::poll(machine::Mctx ctx)
{
    while (true) {
        auto [s, m] = co_await ctx.recv();
        if (auto mr = std::any_cast<mem::MemoryRead>(&m)) {
            if (mr->at >= this->m_mem.size()) {
                co_await ctx.send(s, mem::MemoryFail(mr->at));
            } else {
                co_await ctx.send(s, mem::MemoryResponse(m_mem[mr->at]));
            }
        } else if (auto mw = std::any_cast<mem::MemoryWrite>(&m)) {
            if (mw->at >= this->m_mem.size()) {
                co_await ctx.send(s, mem::MemoryFail(mw->at));
            } else {
                co_await ctx.pause();
                this->m_mem[mw->at] = mw->val;
            }
        }
    }
}
auto Memory::mem() -> mem_t&
{
    return m_mem;
}
auto Memory::mem() const -> const mem_t&
{
    return m_mem;
}
}
