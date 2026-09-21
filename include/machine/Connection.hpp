#ifndef CONNECTION_HPP
#define CONNECTION_HPP
#include "Pollable.hpp"
#include <concepts>
namespace machine {
class Component;
class Connection : public Pollable {
protected:
    Component *start { }, *end { };
    std::string in { }, out { };

private:
    std::string m_name;

public:
    Connection(
        std::string name,
        Component* start,
        Component* end,
        std::string in,
        std::string out);
    Connection() = delete;
    virtual ~Connection();
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    inline Connection(Connection&& o)
        : start { o.start }
        , end { o.end }
        , in { std::move(o.in) }
        , out { std::move(o.out) }
        , m_name { std::move(o.m_name) }
    {
        o.start = nullptr;
        o.end = nullptr;
    }
    inline Connection& operator=(Connection&& o)
    {
        start = o.start;
        end = o.end;
        in = o.in;
        out = o.out;
        m_name = std::move(o.m_name);
        o.start = nullptr;
        o.end = nullptr;
        return *this;
    }
    const Component* get_start() const;
    const Component* get_end() const;
    template <std::derived_from<Component> T>
    const std::optional<T*> get_start_as() const
    {
        if (auto* c = dynamic_cast<T*>(get_start()); c) {
            return c;
        } else {
            return { };
        }
    }
    template <std::derived_from<Component> T>
    const std::optional<T*> get_end_as() const
    {
        if (auto* c = dynamic_cast<T*>(get_end()); c) {
            return c;
        } else {
            return { };
        }
    }
    const std::string& get_name() const;
    virtual std::pair<std::any, std::function<void(std::any)>> on_connecting_to_start();
    virtual std::pair<std::any, std::function<void(std::any)>> on_connecting_to_end();
};

}
#endif
