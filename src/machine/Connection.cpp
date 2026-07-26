#include "machine/MachineGraph.hpp"
Connection::Connection(Component* start, Component* end)
    : start { start }
    , end { end }
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
