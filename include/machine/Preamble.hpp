#pragma once
#include <concepts>

namespace machine {
class Component;
class Connection;
// template <
//     std::derived_from<Component> Comp,
//     std::derived_from<Connection> Conn>
// class MachineGraph;
class MachineContext;
class Sheduler;
class Pollable;
}
