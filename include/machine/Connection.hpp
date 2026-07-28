#ifndef CONNECTION_HPP
#define CONNECTION_HPP
#include "Pollable.hpp"
namespace machine {
class Component;
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

    virtual std::pair<std::any, std::function<void(std::any)>> on_connecting_to_start();
    virtual std::pair<std::any, std::function<void(std::any)>> on_connecting_to_end();
};

}
#endif
