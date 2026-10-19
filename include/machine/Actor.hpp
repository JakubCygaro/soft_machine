#ifndef ACTOR_HPP
#define ACTOR_HPP
#include <coroutine>
#include <exception>
#include <optional>
#include <utility>
namespace machine::actor {

struct promise_base {

protected:
    std::coroutine_handle<> m_continuation = nullptr;
    std::exception_ptr m_exception = nullptr;

public:
    inline void set_continuation(std::coroutine_handle<>&& cont) noexcept
    {
        m_continuation = cont;
    };
    inline void unhandled_exception() noexcept
    {
        m_exception = std::current_exception();
    }
    inline const std::exception_ptr& exception() const noexcept
    {
        return m_exception;
    }
};
template <class T>
struct Actor {
    struct Promise;
    using CoroutineHandle = std::coroutine_handle<Promise>;
    struct Promise : public promise_base {
        std::optional<T> m_return_value { };
        inline Actor get_return_object() noexcept
        {
            auto h = CoroutineHandle::from_promise(*this);
            return Actor { h };
        }
        inline std::suspend_never initial_suspend() const noexcept
        {
            return { };
        }
        struct final_awaiter {
            static inline bool await_ready() noexcept
            {
                return false;
            }
            inline std::coroutine_handle<>
            await_suspend(CoroutineHandle h) const noexcept
            {
                if (auto continuation = h.promise().m_continuation; continuation)
                    return continuation;
                else
                    return std::noop_coroutine();
            }
            static inline void await_resume() noexcept { }
        };
        static inline final_awaiter final_suspend() noexcept
        {
            return { };
        }
        inline void return_value(T&& val) noexcept
        {
            this->m_return_value = std::move(val);
        }
    };
    using promise_type = Promise;
    using handle_t = CoroutineHandle;

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
    inline Actor& operator=(Actor&& other) noexcept
    {
        if (this != &other) {
            this->m_couroutine = std::move(other.m_couroutine);
            other.m_couroutine = nullptr;
        }
        return *this;
    }
    static inline bool await_ready() noexcept { return false; }
    inline bool await_suspend(std::coroutine_handle<> continuation) noexcept
    {
        m_couroutine
            .promise()
            .set_continuation(std::move(continuation));
        return true;
    }
    inline T await_resume() const
    {
        if (auto ex = m_couroutine.promise().exception(); ex) {
            std::rethrow_exception(ex);
        }
        return *m_couroutine.promise().m_return_value;
    }
    inline void manual_resume()
    {
        if (m_couroutine && !m_couroutine.done()) {
            m_couroutine.resume();
        }
    }

private:
    handle_t m_couroutine;
};
template <>
struct Actor<void> {
    struct Promise;
    using CoroutineHandle = std::coroutine_handle<Promise>;
    struct Promise : public promise_base {
        inline Actor get_return_object() noexcept
        {
            auto h = CoroutineHandle::from_promise(*this);
            return Actor { h };
        }
        static inline std::suspend_never initial_suspend() noexcept { return { }; }
        struct final_awaiter {
            static inline bool await_ready() noexcept { return false; }
            inline std::coroutine_handle<>
            await_suspend(CoroutineHandle h) const noexcept
            {
                if (auto continuation = h.promise().m_continuation; continuation)
                    return continuation;
                else
                    return std::noop_coroutine();
            }
            static inline void await_resume() noexcept { }
        };
        static inline final_awaiter final_suspend() noexcept { return { }; }
        inline static void return_void() noexcept { }
    };
    using promise_type = Promise;
    using handle_t = CoroutineHandle;

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
    inline Actor& operator=(Actor&& other) noexcept
    {
        if (this != &other) {
            this->m_couroutine = std::move(other.m_couroutine);
            other.m_couroutine = nullptr;
        }
        return *this;
    }
    static inline bool await_ready() noexcept { return false; }
    inline bool await_suspend(std::coroutine_handle<> continuation) noexcept
    {
        m_couroutine.promise().set_continuation(std::move(continuation));
        return true;
    }
    inline void await_resume() const
    {
        if (auto ex = m_couroutine.promise().exception(); ex) {
            std::rethrow_exception(ex);
        }
    }
    // manual resume done by schedulers and other non-coroutines
    inline void manual_resume()
    {
        if (m_couroutine && !m_couroutine.done()) {
            m_couroutine.resume();
        }
    }

private:
    handle_t m_couroutine;
};
}

#endif
