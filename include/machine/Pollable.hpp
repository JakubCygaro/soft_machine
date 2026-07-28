#ifndef POLLABLE_HPP
#define POLLABLE_HPP
#include "machine/Actor.hpp"
#include "machine/MachineContext.hpp"
namespace machine{
    class Pollable {
    public:
        virtual actor::Actor poll(MachineContext) = 0;
    };
}
#endif
