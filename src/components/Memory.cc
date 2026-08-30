#include "components/Memory.hpp"
#include "common/reflect/Enum.hpp"
#include "game/resources/Resources.hpp"
#include <algorithm>
#include <any>
#include <cmath>
#include <ranges>
#include <raylib.h>
#include <raymath.h>
#include <string>
using namespace game::resources;

namespace components {
namespace {
    auto calc_square_side(size_t cell_count) -> size_t
    {
        auto side = static_cast<size_t>(
            std::ceil(
                std::sqrt(
                    static_cast<float>(
                        cell_count))));
        return side;
    }
}
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

    const auto draw_cell_at = [this](
                                  const ::Vector2& pos,
                                  const std::string& integer,
                                  const ::Vector2& cell_dims) {
        auto cell_center = ::Vector2Add(
            pos,
            ::Vector2Scale(
                m_cell_sz,
                0.5));

        auto int_pos = ::Vector2Subtract(
            cell_center,
            ::Vector2Scale(
                cell_dims,
                0.5));
        ::DrawRectangleRec(
            {
                .x = pos.x,
                .y = pos.y,
                .width = m_cell_sz.x,
                .height = m_cell_sz.y,
            },
            MEM_CELL_COLOR);
        ::DrawRectangleLinesEx(
            {
                .x = pos.x,
                .y = pos.y,
                .width = m_cell_sz.x,
                .height = m_cell_sz.y,
            },
            1.0, DATA_COLOR);
        ::DrawTextEx(
            get_node_font(),
            integer.c_str(),
            int_pos,
            default_node_font_size(),
            default_font_spacing(),
            DATA_COLOR);
    };

    switch (m_lay) {
    case Layout::Square: {
        auto side = calc_square_side(m_mem_strs.size());
        auto cell_reg = ::Vector2 {
            .x = side * m_cell_sz.x,
            .y = side * m_cell_sz.y,
        };
        auto cell_start = ::Vector2Subtract(
            center,
            ::Vector2Scale(
                cell_reg,
                0.5));
        ::DrawRectangleRec(
            {
                .x = cell_start.x,
                .y = cell_start.y,
                .width = cell_reg.x,
                .height = cell_reg.y,
            },
            MEM_CELL_COLOR);
        auto s = m_mem_strs.begin();
        for (auto i = 0UL; i < side; i++) {
            for (auto j = 0UL; j < side; j++) {
                auto pos = ::Vector2 {
                    .x = cell_start.x + (j * m_cell_sz.x),
                    .y = cell_start.y + (i * m_cell_sz.y),
                };
                auto cell_center = ::Vector2Add(
                    pos,
                    ::Vector2Scale(
                        m_cell_sz,
                        0.5));

                if (s >= m_mem_strs.end()) {
                    ::DrawRectangleRec(
                        {
                            .x = pos.x,
                            .y = pos.y,
                            .width = m_cell_sz.x,
                            .height = m_cell_sz.y,
                        },
                        EMPTY_CELL);
                    continue;
                }
                auto cell_dims = std::get<::Vector2>(*s);
                auto int_pos = ::Vector2Subtract(
                    cell_center,
                    ::Vector2Scale(
                        cell_dims,
                        0.5));
                ::DrawRectangleLinesEx(
                    {
                        .x = pos.x,
                        .y = pos.y,
                        .width = m_cell_sz.x,
                        .height = m_cell_sz.y,
                    },
                    1.0, DATA_COLOR);
                ::DrawTextEx(
                    get_node_font(),
                    std::get<std::string>(*s++).c_str(),
                    int_pos,
                    default_node_font_size(),
                    default_font_spacing(),
                    DATA_COLOR);
            }
        }
    } break;
    case Layout::Horizontal: {
        auto column = 0;
        ::Vector2 cell_start = {
            .x = center.x - m_mem_strs.size() * m_cell_sz.x / 2.0f, // + MARGIN / 2.0f,
            .y = center.y - m_cell_sz.y / 2.0f // + MARGIN / 2.0f
        };
        for (const auto& [i, dim] : m_mem_strs) {
            auto pos = ::Vector2 {
                .x = cell_start.x + (column++ * m_cell_sz.x),
                .y = cell_start.y,
            };
            draw_cell_at(pos, i, dim);
        }
    } break;
    case Layout::Vertical: {
        auto row = 0;
        ::Vector2 cell_start = {
            .x = center.x - m_cell_sz.x / 2.0f, // + MARGIN / 2.0f
            .y = center.y - m_mem_strs.size() * m_cell_sz.y / 2.0f, // + MARGIN / 2.0f,
        };
        for (const auto& [i, dim] : m_mem_strs) {
            auto pos = ::Vector2 {
                .x = cell_start.x,
                .y = cell_start.y + (row++ * m_cell_sz.y),
            };
            draw_cell_at(pos, i, dim);
        }
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
std::pair<::Vector2, std::vector<Memory::cell_str_dim>>
Memory::setup_mem(const mem_t& mem)
{
    std::vector<Memory::cell_str_dim> out { };
    ::Vector2 sz = { };
    const auto measure_sz = [&](const std::string& s) {
        auto m = ::MeasureTextEx(
            get_node_font(),
            s.c_str(),
            default_node_font_size(),
            default_font_spacing());
        sz.x = std::max(m.x * 2.2f, sz.x);
        sz.y = std::max(m.y * 2.2f, sz.y);
        out.push_back({ s, m });
    };
    std::for_each(mem.begin(), mem.end(), [&](auto integer) {
        measure_sz(std::to_string(integer));
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
        const auto side = calc_square_side(ints.size());
        self.m_bounds.width = def.x + MARGIN + self.m_name_sz.x + cell.x * side;
        self.m_bounds.height = def.y + MARGIN + self.m_name_sz.y + cell.y * side;
    } break;
    case Memory::Layout::Vertical: {
        self.m_bounds.width = def.x + MARGIN + self.m_name_sz.x + cell.x;
        self.m_bounds.height = def.y + MARGIN + self.m_name_sz.y + cell.y * ints.size();
    } break;
    case Memory::Layout::Horizontal: {
        self.m_bounds.width = def.x + MARGIN + self.m_name_sz.x + cell.x * ints.size();
        self.m_bounds.height = def.y + MARGIN + self.m_name_sz.y + cell.y;
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
const char*
Memory::marshall_to_xml_name() const noexcept
{
    return "memory";
}
void Memory::marshall_to_xml(pugi::xml_node& self) const noexcept
{
    OComponent::marshall_to_xml(self);
    self.append_attribute("layout")
        .set_value(common::reflect::enum_to_string(this->m_lay));
    auto memset = self.append_child("memset");
    std::string memset_str;
    std::ranges::for_each(
        std::views::zip(
            std::views::transform(
                this->m_mem,
                [](const int& i) {
                    return std::to_string(i);
                }),
            std::views::iota(1)),
        [&](const auto& tp) {
            const auto [s, i] = tp;
            memset_str.append(s);
            if (i != static_cast<int>(m_mem.size()))
                memset_str.push_back(' ');
        });
    memset.text().set(memset_str);
}
}
