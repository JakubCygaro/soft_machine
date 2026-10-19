#ifndef ACTOR_HPP
#define ACTOR_HPP
#include <coroutine>
#include <print>
#include <utility>
namespace machine::actor {

template <class T = void>
struct Actor {
    struct Promise;
    using CoroutineHandle = std::coroutine_handle<Promise>;
    struct promise_base {
        friend Actor;

    protected:
        CoroutineHandle m_continuation = nullptr;
        std::optional<T> m_return_value { };

    public:
        inline void set_continuation(CoroutineHandle&& cont)
        {
            m_continuation = cont;
        };
    };
    struct Promise : public promise_base {
        inline Actor get_return_object()
        {
            auto h = CoroutineHandle::from_promise(*this);
            std::println("get_return_object {}", h.address());
            return Actor { h };
        }
        inline std::suspend_never initial_suspend() noexcept
        {
            std::println("promise initial_suspend {}",
                CoroutineHandle::from_promise(*this).address());
            return { };
        }
        struct final_awaiter {
            CoroutineHandle continuation = nullptr;
            inline bool await_ready() const noexcept
            {
                std::println("final_suspend await_ready");
                return false;
            }
            inline std::coroutine_handle<>
            await_suspend(CoroutineHandle h) const noexcept
            {
                std::println("final_suspend await_suspend {}", h.address());
                if (auto continuation = h.promise().m_continuation; continuation)
                    return continuation;
                else
                    return std::noop_coroutine();
            }

            inline void await_resume() const noexcept
            {
                std::println("final_suspend await_resume");
            }
        };
        inline final_awaiter final_suspend() noexcept
        {
            std::println("promise final_suspend {}",
                CoroutineHandle::from_promise(*this).address());
            return { };
        }
        [[noreturn]]
        inline static void unhandled_exception()
        {
            throw;
        }
        inline void return_value(T&& val) noexcept
        {
            this->m_return_value = std::move(val);
            std::println("promise return_value");
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
    inline bool await_ready()
    {
        std::println("Actor await_ready m_cor: {}",
            m_couroutine.address());
        return false;
    }
    inline bool await_suspend(handle_t continuation)
    {
        std::println("Actor await_suspend m_cor: {} continuation: {}",
            m_couroutine.address(),
            continuation.address());
        m_couroutine.promise().set_continuation(std::move(continuation));
        return true;
    }
    inline T await_resume()
    {
        std::println("Actor await_resume m_cor: {}",
            m_couroutine.address());
        return *m_couroutine.promise().m_return_value;
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
template <>
struct Actor<void> {
    struct Promise;
    using CoroutineHandle = std::coroutine_handle<Promise>;
    struct promise_base {
    protected:
        CoroutineHandle m_continuation = nullptr;

    public:
        inline void set_continuation(CoroutineHandle&& cont)
        {
            m_continuation = cont;
        };
    };
    struct Promise : public promise_base {
        inline Actor get_return_object()
        {
            auto h = CoroutineHandle::from_promise(*this);
            std::println("get_return_object {}", h.address());
            return Actor { h };
        }
        inline std::suspend_never initial_suspend() noexcept
        {
            std::println("promise initial_suspend {}",
                CoroutineHandle::from_promise(*this).address());
            return { };
        }
        struct final_awaiter {
            CoroutineHandle continuation = nullptr;
            inline bool await_ready() const noexcept
            {
                std::println("final_suspend await_ready");
                return false;
            }
            inline std::coroutine_handle<> await_suspend(CoroutineHandle h) const noexcept
            {
                std::println("final_suspend await_suspend {}", h.address());
                if (auto continuation = h.promise().m_continuation; continuation)
                    return continuation;
                else
                    return std::noop_coroutine();
            }

            inline void await_resume() const noexcept
            {
                std::println("final_suspend await_resume");
            }
        };
        inline final_awaiter final_suspend() noexcept
        {
            std::println("promise final_suspend {}",
                CoroutineHandle::from_promise(*this).address());
            return { };
        }
        [[noreturn]]
        inline static void unhandled_exception()
        {
            throw;
        }
        inline static void return_void() noexcept
        {
            std::println("promise return_void");
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
    inline bool await_ready()
    {
        std::println("Actor await_ready m_cor: {}",
            m_couroutine.address());
        return false;
    }
    inline bool await_suspend(handle_t continuation)
    {
        std::println("Actor await_suspend m_cor: {} continuation: {}",
            m_couroutine.address(),
            continuation.address());
        m_couroutine.promise().set_continuation(std::move(continuation));
        return true;
    }
    inline void await_resume()
    {
        std::println("Actor await_resume m_cor: {}",
            m_couroutine.address());
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
