#include "components/Passthrough.hpp"
namespace components {
void Passthrough::draw()
{
    ::DrawLineBezier(m_start_pos, m_end_pos, 5.f, m_color);
}
machine::actor::Actor Passthrough::poll(machine::Mctx ctx)
{
    while (true) {
        m_color = ::BLACK;
        auto [s, m] = co_await ctx.recv();
        if (s == in)
            co_await ctx.send(out, std::move(m));
        else if (s == out)
            co_await ctx.send(in, std::move(m));
        m_color = ::RED;
        co_await ctx.pause();
    }
}

}
