#include "machine/MachineGraph.hpp"
namespace machine {

void MachineGraph::poll_all()
{
    for (auto i = procs.size(); i > 0; i--) {
        auto p = std::move(procs.front());
        procs.pop_front();
        std::println("resuming");
        std::flush(std::cout);
        if (p.actor.await_ready()) {
            auto res = p.actor.await_resume();
            if(res)
                throw std::runtime_error("Actor exited");
        }
        procs.push_back(std::move(p));
    }
}
}
