#include "machine/Connection.hpp"
#include <utility>
namespace machine {
Connection::Connection(
    std::string name,
    Component* start,
    Component* end,
    std::string in,
    std::string out)
    : start { start }
    , end { end }
    , in { in }
    , out { out }
    , m_name { name }
{
}
const Component* Connection::get_start() const
{
    return this->start;
}
const Component* Connection::get_end() const
{
    return this->end;
}
const std::string& Connection::get_name() const
{
    return m_name;
}

std::pair<std::any, std::function<void(std::any)>>
Connection::on_connecting_to_start()
{
    return std::make_pair(nullptr, [](std::any) { });
}
std::pair<std::any, std::function<void(std::any)>>
Connection::on_connecting_to_end()
{
    return std::make_pair(nullptr, [](std::any) { });
}
Connection::~Connection() { };
}
