#ifndef COMPONENT_HPP
#define COMPONENT_HPP
#include "machine/Connection.hpp"
#include "machine/Pollable.hpp"
#include <string>
namespace machine {

class Component : public Pollable {
private:
    std::string name { };

public:
    Component(std::string name);
    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;
    inline Component(Component&& o)
        : name { std::move(o.name) }
    {
    }
    inline Component& operator=(Component&& o)
    {
        this->name = o.name;
        return *this;
    }
    virtual ~Component();
    const std::string& get_name() const;

    // conn_msg is what the connection object has sent as a message
    // the return value is the response of this component
    // nullptr means no message
    virtual std::any on_incoming_connection(
        std::string_view,
        const Connection*,
        std::any conn_msg);
    virtual std::any on_outcoming_connection(
        std::string_view,
        const Connection*,
        std::any conn_msg);
    ;
};

}

#endif
