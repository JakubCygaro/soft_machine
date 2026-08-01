#include "components/Memory.hpp"
#include <algorithm>
#include <any>
#include <cmath>
#include <string>
namespace components {
void Memory::draw()
{
    ::DrawRectangleRounded(
        this->m_bounds,
        0.5,
        2,
        ::BLUE);
    ::DrawTextEx(
        ::GetFontDefault(),
        m_mem_str.c_str(),
        Vector2 { .x = m_bounds.x, .y = m_bounds.y },
        12,
        10,
        ::BLACK);
}
void Memory::update()
{
}
std::pair<::Vector2, std::string>
Memory::calculate_dims_from_mem(const mem_t& mem)
{
    std::string out { };
    ::Vector2 sz = { };
    auto side = static_cast<size_t>(
        std::sqrt(
            static_cast<float>(
                mem.size())));
    std::string line { };
    auto b = mem.begin();
    const auto measure_append = [&](std::string& l) {
        auto m = ::MeasureTextEx(
            ::GetFontDefault(),
            l.c_str(),
            12,
            10);
        sz.y += m.y;
        sz.x = std::max(sz.x, m.x);
        out.append(l);
        out.append("\n");
    };
    for (size_t i = 0; i < side; i++ && b != mem.end()) {
        for (size_t j = 0; j < side; j++ && b != mem.end()) {
            line.append(std::to_string(*b++));
        }
        measure_append(line);
        line.clear();
    }
    if (!line.empty())
        measure_append(line);
    return std::make_pair(sz, out);
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
