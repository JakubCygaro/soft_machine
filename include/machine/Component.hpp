#ifndef COMPONENT_HPP
#define COMPONENT_HPP
#include "machine/Actor.hpp"
#include "machine/MachineGraph.hpp"
#include <string>
namespace machine {
class Component {
protected:
    const std::string name { };

public:
    std::deque<Message> msgq { };

    explicit Component(std::string name);
    virtual ~Component();
    const std::string& get_name() const;
    virtual actor::Actor<void> poll(MachineGraph::MachineContext ctx) = 0;
};

}

#endif
