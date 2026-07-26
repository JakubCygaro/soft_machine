#ifndef ACTOR_HPP
#define ACTOR_HPP
#include <coroutine>
#include <optional>
namespace machine::actor {
template <typename Ret>
struct Actor {
public:
    struct promise_type {
        Actor<Ret> get_return_object()
        {
            return Actor { handle_t::from_promise(*this) };
        }
        static std::suspend_always initial_suspend() noexcept
        {
            return { };
        }
        static std::suspend_always final_suspend() noexcept
        {
            return { };
        }
        std::suspend_always yield_value(Ret value) noexcept
        {
            current_value = std::move(value);
            return { };
        }
        void return_value(std::optional<Ret> value)
        {
            current_value = std::move(value);
        }
        [[noreturn]]
        static void unhandled_exception()
        {
            throw;
        }
        std::optional<Ret> current_value;
    };
    using handle_t = std::coroutine_handle<promise_type>;

    explicit Actor(const handle_t coroutine)
        : m_couroutine { coroutine }
    {
    }
    Actor() = default;
    ~Actor()
    {
        if (m_couroutine)
            m_couroutine.destroy();
    }
    Actor(const Actor&) = delete;
    Actor& operator=(const Actor&) = delete;

    Actor(const Actor&& other) noexcept
        : m_couroutine { other.m_couroutine }
    {
        other.m_couroutine = { };
    }
    Actor& operator=(const Actor&& other) noexcept
    {
        if (this != &other) {
            if (m_couroutine)
                m_couroutine.destroy();
            this->m_couroutine = other.m_couroutine;
            other.m_couroutine = { };
        }
        return *this;
    }
    std::optional<Ret> resume()
    {
        if (m_couroutine && !m_couroutine.done()) {
            m_couroutine.resume();
            return m_couroutine.promise().current_value;
        }
        return std::nullopt;
    }
    bool await_ready()
    {
        return true;
    }
    bool await_suspend(handle_t h)
    {
        return false;
    }
    std::optional<Ret> await_resume()
    {
        return resume();
    }

private:
    handle_t m_couroutine;
};

}

#endif
