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

}
