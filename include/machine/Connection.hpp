#ifndef CONNECTION_HPP
#define CONNECTION_HPP
#include "machine/MachineGraph.hpp"
namespace machine {
class Connection : public Pollable {
private:
    Component *start { }, *end { };

public:
    Connection(Component* start, Component* end);
    Connection() = delete;
    virtual ~Connection();
    const Component* get_start() const;
    const Component* get_end() const;
};

}
#endif
