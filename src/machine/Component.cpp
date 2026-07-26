#include "machine/MachineGraph.hpp"
Component::Component(std::string name)
    : name { name }
{
}
void Component::add_connection(std::weak_ptr<Connection> conn, std::string name)
{
    named_connections[name] = conn;
}
Component::~Component() { };
const std::string& Component::get_name() const
{
    return name;
}
