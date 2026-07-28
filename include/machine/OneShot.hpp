#ifndef ONESHOT_HPP
#define ONESHOT_HPP
#include <atomic>
#include <concepts>
#include <memory>
#include <optional>
namespace machine {
template <std::movable T>
class OneShot {
    using data_t = std::shared_ptr<
        std::optional<T>>;
    using change_t = std::shared_ptr<
        std::atomic<bool>>;
    OneShot() = delete;

public:
    class Read;
    class Write;
    inline static std::pair<Read, Write> create()
    {
        data_t v = std::make_shared<std::optional<T>>();
        change_t c = std::make_shared<std::atomic<bool>>(false);
        return { Read(v, c), Write(v, c) };
    }
    class Read {
        friend class OneShot<T>;
        data_t m_val;
        change_t m_change;
        inline explicit Read(data_t val, change_t change)
            : m_val { val }
            , m_change { change } { };
        // friend std::pair<Read, Write> create();
        Read(const Read&) = delete;
        Read& operator=(const Read&) = delete;

    public:
        inline Read(Read&& other)
            : m_val { other.m_val }
            , m_change { other.m_change }
        {
            other.m_val = nullptr;
            other.m_change = nullptr;
        }
        inline Read& operator=(Read&& other)
        {
            this->m_val = other.m_val;
            this->m_change = other.m_change;
            other.m_val = nullptr;
            other.m_change = nullptr;
            return *this;
        }
        inline T recv() const
        {
            if (m_change->load())
                throw std::runtime_error("attempted to call recv multiple times");
            m_change->wait(false);
            auto v = std::move(**m_val);
            (*m_val).reset();
            return v;
        }
    };

    class Write {
        friend class OneShot<T>;
        data_t m_val;
        change_t m_change;
        inline explicit Write(data_t val, change_t change)
            : m_val { val }
            , m_change { change }
        {
        }
        // friend std::pair<Read, Write> create();
        Write(const Write&) = delete;
        Write& operator=(const Write&) = delete;

    public:
        inline Write(Write&& other)
            : m_val { other.m_val }
            , m_change { other.m_change }
        {
            other.m_val = nullptr;
            other.m_change = nullptr;
        }
        inline Write& operator=(Write&& other)
        {
            this->m_val = other.m_val;
            this->m_change = other.m_change;
            other.m_val = nullptr;
            other.m_change = nullptr;
            return *this;
        }
        inline void send(T&& v)
        {
            if (m_change->load()) {
                throw std::runtime_error("oneshot already written");
            }
            m_val->store(std::move(v));
            m_change->store(true);
        }
    };
};
}
#endif
