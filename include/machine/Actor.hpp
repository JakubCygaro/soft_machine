#ifndef ACTOR_HPP
#define ACTOR_HPP
#include <algorithm>
#include <coroutine>
namespace machine::actor {
struct Actor {
public:
    struct promise_type {
        inline Actor get_return_object()
        {
            return Actor { handle_t::from_promise(*this) };
        }
        inline static std::suspend_always initial_suspend() noexcept
        {
            return { };
        }
        inline static std::suspend_always final_suspend() noexcept
        {
            return { };
        }
        inline std::suspend_always yield_value() noexcept
        {
            return { };
        }
        [[noreturn]]
        inline static void unhandled_exception()
        {
            throw;
        }
        inline static void return_void() noexcept { }
    };
    using handle_t = std::coroutine_handle<promise_type>;

    inline explicit Actor(const handle_t coroutine)
        : m_couroutine { coroutine }
    {
    }
    inline Actor() = default;
    inline ~Actor()
    {
        if (m_couroutine)
            m_couroutine.destroy();
    }
    inline Actor(const Actor&) = delete;
    inline Actor& operator=(const Actor&) = delete;

    inline Actor(Actor&& other) noexcept
        : m_couroutine { std::move(other.m_couroutine) }
    {
        other.m_couroutine = nullptr;
    }
    inline Actor& operator=(const Actor&& other) noexcept
    {
        if (this != &other) {
            if (m_couroutine)
                m_couroutine.destroy();
            this->m_couroutine = std::move(other.m_couroutine);
        }
        return *this;
    }
    inline bool resume()
    {
        if (m_couroutine && !m_couroutine.done()) {
            m_couroutine.resume();
            return true;
        }
        return false;
    }
    inline bool await_ready()
    {
        return true;
    }
    inline bool await_suspend(handle_t h)
    {
        (void)h;
        return false;
    }
    inline bool await_resume()
    {
        return resume();
    }

private:
    handle_t m_couroutine;
};

}

#endif
