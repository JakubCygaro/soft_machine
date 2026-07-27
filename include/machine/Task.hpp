#ifndef TASK_HPP
#define TASK_HPP
#include <algorithm>
#include <concepts>
#include <coroutine>
#include <type_traits>

namespace machine::task {

template <typename Ret>
struct Task {
public:
    struct promise_type;
    using handle_t = std::coroutine_handle<promise_type>;
    using return_t = Ret;

private:
    handle_t m_couroutine = nullptr;

public:
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
    inline Task(const Task&& other) noexcept
        : m_couroutine { other.m_couroutine }
    {
        other.m_couroutine = { };
    }
    inline Task& operator=(const Task&& other) noexcept
    {
        if (this != &other) {
            if (m_couroutine)
                m_couroutine.destroy();
            this->m_couroutine = other.m_couroutine;
            other.m_couroutine = { };
        }
        return *this;
    }
    inline return_t resume()
    {
        if (m_couroutine && !m_couroutine.done()) {
            m_couroutine.resume();
            if constexpr (std::is_same_v<return_t, void>) {
                return;
            } else {
                return m_couroutine.promise().current_value;
            }
        }
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
    inline return_t await_resume()
    {
        return resume();
    }
    struct promise_type {
        inline Task<return_t> get_return_object()
        {
            return Task { handle_t::from_promise(*this) };
        }
        inline static std::suspend_never initial_suspend() noexcept
        {
            return { };
        }
        inline static std::suspend_never final_suspend() noexcept
        {
            return { };
        }
        inline std::suspend_never yield_value(return_t value) noexcept
        {
            current_value = std::move(value);
            return { };
        }
        inline void return_value(std::optional<return_t> value)
        {
            current_value = std::move(value);
        }
        // inline void return_void() noexcept { }
        [[noreturn]]
        inline static void unhandled_exception()
        {
            throw;
        }
        std::optional<return_t> current_value;
    };
};
}
#endif
