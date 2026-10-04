#pragma once
#include <concepts>
#include <coroutine>
#include <optional>
#include <utility>

namespace machine::task {

template <class T>
struct Task {
public:
    struct promise_type {
        friend Task;

    private:
        std::optional<T> m_ret_val { };

    public:
        inline Task get_return_object()
        {
            return Task { handle_t::from_promise(*this) };
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
        inline void return_value(T&& v) noexcept
        {
            m_ret_val = v;
        }
    };
    using handle_t = std::coroutine_handle<promise_type>;

    inline explicit Task(const handle_t coroutine)
        : m_couroutine { coroutine }
    {
    }
    inline Task() = default;
    inline ~Task()
    {
        if (m_couroutine)
            m_couroutine.destroy();
    }
    inline Task(const Task&) = delete;
    inline Task& operator=(const Task&) = delete;

    inline Task(Task&& other) noexcept
        : m_couroutine { std::move(other.m_couroutine) }
    {
        other.m_couroutine = nullptr;
    }
    inline Task& operator=(Task&& other) noexcept
    {
        if (this != &other) {
            if (m_couroutine)
                m_couroutine.destroy();
            this->m_couroutine = std::move(other.m_couroutine);
            other.m_couroutine = nullptr;
        }
        return *this;
    }
    inline bool await_ready() { return false; }
    inline void await_suspend(handle_t h)
    {
        (void)h;
    }
    inline T await_resume()
    {
        return std::move(*m_couroutine.m_ret_val);
    }

private:
    handle_t m_couroutine;
};
template <>
class Task<void> {
public:
    struct promise_type {
        friend Task;

    public:
        inline Task get_return_object()
        {
            return Task { handle_t::from_promise(*this) };
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
        inline static void return_void() noexcept
        {
        }
    };
    using handle_t = std::coroutine_handle<promise_type>;

    inline explicit Task(const handle_t coroutine)
        : m_couroutine { coroutine }
    {
    }
    inline Task() = default;
    inline ~Task()
    {
        if (m_couroutine)
            m_couroutine.destroy();
    }
    inline Task(const Task&) = delete;
    inline Task& operator=(const Task&) = delete;

    inline Task(Task&& other) noexcept
        : m_couroutine { std::move(other.m_couroutine) }
    {
        other.m_couroutine = nullptr;
    }
    inline Task& operator=(Task&& other) noexcept
    {
        if (this != &other) {
            if (m_couroutine)
                m_couroutine.destroy();
            this->m_couroutine = std::move(other.m_couroutine);
            other.m_couroutine = nullptr;
        }
        return *this;
    }
    inline bool await_ready() { return false; }
    inline void await_suspend(handle_t h)
    {
        (void)h;
    }
    inline void await_resume() { }

private:
    handle_t m_couroutine;
};
}
