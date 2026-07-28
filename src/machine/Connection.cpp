#include "machine/Connection.hpp"
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
Connection::~Connection() { };
}
