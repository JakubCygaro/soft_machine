#include "components/Passthrough.hpp"
namespace components {
Passthrough::Passthrough(
    std::string name,
    machine::Component* a,
    machine::Component* b,
    std::string in,
    std::string out)
    : components::OConnection(name, a, b, in, out)
{
}
Passthrough::Passthrough(
    fl::string_param_t name,
    machine::Component* a,
    machine::Component* b,
    fl::string_param_t from,
    fl::string_param_t to)
    : components::OConnection(
          std::string(name.data()), a, b,
          std::string(from.data()), std::string(to.data()))
{
}
Passthrough::Passthrough(Passthrough&& o)
    : components::OConnection(std::move(o))
    , m_color { o.m_color }
{
}
Passthrough& Passthrough::operator=(Passthrough&& o)
{
    if (&o == this)
        return *this;
    components::OConnection::operator=(std::move(o));
    m_color = o.m_color;
    return *this;
}
Passthrough::~Passthrough() { }
void Passthrough::draw()
{
    ::DrawLineBezier(m_start_pos, m_end_pos, 5.f, m_color);
}
machine::actor::Actor<void> Passthrough::poll(machine::Mctx ctx)
{
    while (true) {
        auto [s, m] = co_await ctx.recv();
        m_color = ::RED;
        if (s == in)
            co_await ctx.send(out, std::move(m));
        else if (s == out)
            co_await ctx.send(in, std::move(m));
        co_await ctx.pause();
        m_color = ::BLACK;
    }
}
const char*
Passthrough::marshall_to_xml_name() const noexcept
{
    return "passthrough";
}
}
