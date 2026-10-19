#pragma once

#include <string>
namespace components::msg {
// a common graph message type interface
struct Msg {
    inline virtual ~Msg() { }
    virtual std::string type_name() const = 0;
    virtual std::string to_display_string() const = 0;
};
}
