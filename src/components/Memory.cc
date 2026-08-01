#include "components/Memory.hpp"
#include <any>
namespace components {
void Memory::draw()
{
    ::DrawRectangleRounded(
        this->m_bounds,
        1.0,
        10,
        ::BLUE);
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
