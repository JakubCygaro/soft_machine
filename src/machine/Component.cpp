#include "machine/Component.hpp"
namespace machine {
Component::Component(std::string name)
    : name { name }
{
}
Component::~Component() { };
const std::string& Component::get_name() const
{
    return name;
}
std::any Component::on_incoming_connection(
    std::string_view,
    const Connection*,
    std::any)
{
    return nullptr;
}
std::any Component::on_outcoming_connection(
    std::string_view,
    const Connection*,
    std::any)
{
    return nullptr;
}

}
