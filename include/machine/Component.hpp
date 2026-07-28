#ifndef COMPONENT_HPP
#define COMPONENT_HPP
#include "machine/Connection.hpp"
#include "machine/Pollable.hpp"
#include <string>
namespace machine {

class Component : public Pollable {
protected:
    const std::string name { };

public:
    Component(std::string name);
    virtual ~Component();
    const std::string& get_name() const;
    virtual std::any on_incoming_connection(
        std::string_view,
        const Connection*,
        std::any);
    virtual std::any on_outcoming_connection(
        std::string_view,
        const Connection*,
        std::any);
    ;
};

}

#endif
