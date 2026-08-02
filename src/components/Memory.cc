#include "components/Memory.hpp"
#include "game/resources/Resources.hpp"
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
    ::DrawTextEx(
        get_node_font(),
        m_mem_str.c_str(),
        mem_pos,
        default_node_font_size(),
        default_font_spacing(),
        DATA_COLOR);
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
std::pair<::Vector2, std::string>
Memory::setup_mem(const mem_t& mem)
{
    std::string out { };
    auto side = static_cast<size_t>(
        std::sqrt(
            static_cast<float>(
                mem.size())));
    std::string line { };
    auto b = mem.begin();
    for (size_t i = 0; i < side; i++ && b != mem.end()) {
        for (size_t j = 0; j < side; j++ && b != mem.end()) {
            line.append(std::to_string(*b++));
            line.append(",");
        }
        out.append(line);
        out.append("\n");
        line.clear();
    }
    out.pop_back();
    out.pop_back();
    if (!line.empty()) {
        out.append(line);
        out.pop_back();
    }
    auto m = ::MeasureTextEx(
        get_node_font(),
        out.c_str(),
        default_node_font_size(),
        default_font_spacing());
    return std::make_pair(m, out);
}
void Memory::setup(Memory& self)
{
    auto [dims, txt] = setup_mem(self.m_mem);
    constexpr const auto def = game::resources::get_default_node_size();
    self.m_mem_sz = dims;
    self.m_name_sz = ::MeasureTextEx(
        get_node_font(),
        self.get_name().c_str(),
        default_node_font_size(),
        default_font_spacing());
    self.m_bounds.width = std::max(def.x + 40.0f + self.m_name_sz.x, dims.x);
    self.m_bounds.height = std::max(def.y + 40.0f + self.m_name_sz.y, dims.y);
    self.m_mem_str = txt;
}
machine::actor::Actor Memory::poll(machine::Mctx ctx)
{
    while (true) {
        auto [s, m] = co_await ctx.recv();
        if (auto mr = std::any_cast<mem::MemoryRequest>(&m)) {
            if (mr->at >= this->m_mem.size()) {
                co_await ctx.send(s, mem::MemoryFail(mr->at));
            } else {
                co_await ctx.send(s, mem::MemoryResponse(m_mem[mr->at]));
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
