#include "machine/MachineGraph.hpp"
void MachineGraph::poll_all()
{
    while (!msgq.empty()) {
        auto m = msgq.front();
        msgq.pop_front();
        if (this->named_components.contains(m.receiver)) {
            named_components[m.receiver]->msgq.push_back(std::move(m));
        } else if (this->named_connections.contains(m.receiver)) {
            named_connections[m.receiver]->msgq.push_back(std::move(m));
        }
    }

    std::deque<Component*> comp_polled { };
    std::string name("bullshit");
    while (!compq.empty()) {
        auto c = compq.front();
        compq.pop_front();
        c->poll(MachineContext(&this->msgq));
        comp_polled.push_back(c);
    }
    compq = comp_polled;
    std::deque<Connection*> conn_polled { };
    while (!connq.empty()) {
        auto c = connq.front();
        connq.pop_front();
        c->poll(MachineContext(&this->msgq));
        conn_polled.push_back(c);
    }
    connq = conn_polled;
}
