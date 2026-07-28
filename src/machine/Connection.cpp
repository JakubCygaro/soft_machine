#include "machine/Connection.hpp"
#include <utility>
namespace machine {
Connection::Connection(
    Component* start,
    Component* end,
    std::string in,
    std::string out)
    : start { start }
    , end { end }
    , in { in }
    , out { out }
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
