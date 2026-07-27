#ifndef CONNECTION_HPP
#define CONNECTION_HPP
#include "machine/MachineGraph.hpp"
#include "machine/Actor.hpp"
namespace machine {
class Connection {
private:
    Component *start { }, *end { };

public:
    Connection(Component* start, Component* end);
    Connection() = delete;
    virtual ~Connection();
    const Component* get_start() const;
    const Component* get_end() const;
    virtual actor::Actor poll(MachineGraph::MachineContext ctx) = 0;
};

}
#endif
