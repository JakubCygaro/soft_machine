#ifndef COMPONENT_HPP
#define COMPONENT_HPP
#include "machine/MachineGraph.hpp"
#include <string>
namespace machine {

class Component : public Pollable {
protected:
    const std::string name { };

public:
    Component(std::string name);
    virtual ~Component();
    const std::string& get_name() const;
};

}

#endif
