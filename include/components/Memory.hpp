#pragma once
#include "components/GameGraphElements.hpp"
#include "components/msg/Message.hpp"
#include "machine/Actor.hpp"
#include "machine/MachineContext.hpp"
#include <format>
#include <raylib.h>
#include <string>

namespace components {
namespace mem {
    struct MemoryRequest : public components::msg::Msg {
        inline MemoryRequest() = delete;
        inline MemoryRequest(size_t at)
            : at { at }
        {
        }
        size_t at;
        inline virtual std::string type_name() const override
        {
            return "Memory read request message";
        }
        inline virtual std::string to_display_string() const override
        {
            return std::format("MemRead {{ at: {} }}", at);
        }
    };
    struct MemoryResponse : public components::msg::Msg {
        inline MemoryResponse() = delete;
        inline MemoryResponse(int val)
            : val { val }
        {
        }
        int val;
        inline virtual std::string type_name() const override
        {
            return "Memory read response message";
        }
        inline virtual std::string to_display_string() const override
        {
            return std::format("MemRead {{ val: {} }}", val);
        }
    };
    struct MemoryFail : public components::msg::Msg {
        inline MemoryFail() = delete;
        inline MemoryFail(size_t at)
            : at { at }
        {
        }
        size_t at;
        inline virtual std::string type_name() const override
        {
            return "Memory read failure message";
        }
        inline virtual std::string to_display_string() const override
        {
            return std::format("MemReadFail {{ at: {} }}", at);
        }
    };
}
class Memory : public components::OComponent {
public:
    using mem_t = std::vector<int>;

protected:
    mem_t m_mem;

public:
    inline Memory(
        std::string name,
        mem_t&& mem)
        : components::OComponent(name)
        , m_mem(std::move(mem))
    {
    }
    inline virtual ~Memory() { };

    virtual void draw() override;
    virtual machine::actor::Actor poll(machine::Mctx) override;

    virtual auto mem() -> mem_t&;
    virtual auto mem() const -> const mem_t&;
};
}
