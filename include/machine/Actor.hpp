#ifndef ACTOR_HPP
#define ACTOR_HPP
#include <coroutine>
#include <optional>
#include <print>
#include <utility>
namespace machine::actor {

struct Actor {
public:
    struct promise_type {
        std::optional<std::coroutine_handle<promise_type>> m_continuation { };
        inline Actor get_return_object()
        {
            auto h = handle_t::from_promise(*this);
            std::println("get_return_object {}", h.address());
            return Actor { h };
        }
        struct initial_suspend {
            inline bool await_ready() const noexcept
            {
                std::println("initial_suspend await_ready");
                return true;
            }
            inline void await_suspend(std::coroutine_handle<>) const noexcept
            {
                std::println("initial_suspend await_suspend");
            }

            inline void await_resume() const noexcept
            {
                std::println("initial_suspend await_resume");
            }
        };
        // because this always suspends, the coroutine does
        // not start running before the calling awaiter is already resumed
        inline initial_suspend initial_suspend() noexcept
        {
            std::println("promise initial_suspend {}",
                    handle_t::from_promise(*this).address());
            return { };
        }
        struct final_suspend {
            std::coroutine_handle<promise_type> continuation = nullptr;
            inline bool await_ready() const noexcept
            {
                std::println("final_suspend await_ready");
                return false;
            }
            inline void await_suspend(std::coroutine_handle<> h) const noexcept
            {
                std::println("final_suspend await_suspend {}", h.address());
                if(continuation) return continuation.resume();
            }

            inline void await_resume() const noexcept
            {
                std::println("final_suspend await_resume");
            }
        };
        // TODO: this final suspend must resume the continuation if there is any
        //  the problem is still to actually obtain that continuation
        inline final_suspend final_suspend() noexcept
        {
            std::println("promise final_suspend {}",
                    handle_t::from_promise(*this).address());
            struct final_suspend fs{};
            if(m_continuation)
                fs.continuation = *m_continuation;
            return fs;
        }
        [[noreturn]]
        inline static void unhandled_exception()
        {
            throw;
        }
        inline static void return_void() noexcept {
            std::println("promise return_void");
        }
        // auto await_transform(Actor&& act) {
        //     this
        // }
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
    inline Actor& operator=(Actor&& other) noexcept
    {
        if (this != &other) {
            this->m_couroutine = std::move(other.m_couroutine);
            other.m_couroutine = nullptr;
        }
        return *this;
    }
    inline void resume()
    {
        if (m_couroutine && !m_couroutine.done()) {
            m_couroutine.resume();
            // return true;
        }
        // return false;
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
        m_couroutine.promise().m_continuation = continuation;
        return true;
    }
    inline void await_resume()
    {
        std::println("Actor await_resume m_cor: {}",
                m_couroutine.address());
        return resume();
    }

private:
    handle_t m_couroutine;
};

}

#endif
