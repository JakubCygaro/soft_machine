#ifndef CONNECTION_HPP
#define CONNECTION_HPP
#include "machine/MachineGraph.hpp"
namespace machine {
class Connection : public Pollable {
protected:
    Component *start { }, *end { };
    std::string in { }, out { };

public:
    Connection(Component* start,
        Component* end,
        std::string in,
        std::string out);
    Connection() = delete;
    virtual ~Connection();
    const Component* get_start() const;
    const Component* get_end() const;
};

}
#endif
