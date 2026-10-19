#pragma once
namespace components {
template <typename T>
struct AutoMoveD {
private:
    T val;

public:
    inline AutoMoveD(T&& v)
        : val { v }
    {
    }
    inline AutoMoveD(const T& v)
        : val { v }
    {
    }
    AutoMoveD() = default;
    AutoMoveD(const AutoMoveD&) = delete;
    AutoMoveD& operator=(const AutoMoveD&) = delete;
    AutoMoveD(AutoMoveD&&) = default;
    AutoMoveD& operator=(AutoMoveD&&) = default;
    inline T* operator->() noexcept
    {
        return &val;
    }
    inline const T* operator->() const noexcept
    {
        return &val;
    }
};
}
