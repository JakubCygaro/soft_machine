#include "components/Passthrough.hpp"
namespace components {
void Passthrough::draw()
{
    ::DrawLineBezier(m_start_pos, m_end_pos, 5.f, m_color);
}
machine::actor::Actor Passthrough::poll(machine::Mctx ctx)
{
    while (true) {
        auto [s, m] = co_await ctx.recv();
        m_color = ::RED;
        if (s == in)
            co_await ctx.send(out, std::move(m));
        else if (s == out)
            co_await ctx.send(in, std::move(m));
        m_color = ::BLACK;
        co_await ctx.pause();
    }
}
const char*
Passthrough::marshall_to_xml_name() const noexcept
{
    return "passthrough";
}
// void Passthrough::marshall_to_xml(pugi::xml_node&) const noexcept
// {
//
// }
}
