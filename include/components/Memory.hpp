#pragma once
#include "components/GameGraphElements.hpp"
#include "components/msg/Message.hpp"
#include "game/resources/Resources.hpp"
#include "machine/Actor.hpp"
#include "machine/MachineContext.hpp"
#include <format>
#include <raylib.h>
#include <string>

namespace components {
namespace mem {
    struct MemoryWrite : public components::msg::Msg {
        inline MemoryWrite() = delete;
        inline MemoryWrite(size_t at, int val)
            : at { at }
            , val { val }
        {
        }
        size_t at;
        int val;
        inline virtual std::string type_name() const override
        {
            return "Memory write request message";
        }
        inline virtual std::string to_display_string() const override
        {
            return std::format("MemWrite {{ at: {} val {} }}", at, val);
        }
    };
    struct MemoryRead : public components::msg::Msg {
        inline MemoryRead() = delete;
        inline MemoryRead(size_t at)
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
    const Color BODY_COLOR = ::BLUE;
    const Color DATA_COLOR = ::BLACK;
    const Color MEM_CELL_COLOR = ::WHITE;
    enum class Layout {
        Square,
        Vertical,
        Horizontal,
    };

public:
    using mem_t = std::vector<int>;

protected:
    mem_t m_mem;
    std::vector<std::string> m_mem_strs;
    ::Vector2 m_mem_sz;
    ::Vector2 m_name_sz;
    Layout m_lay { Layout::Square };
    ::Vector2 m_cell_sz;

public:
    inline Memory(
        std::string name,
        mem_t&& mem)
        : components::OComponent(name)
        , m_mem(std::move(mem))
    {
        setup(*this);
    }
    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;
    inline Memory(Memory&& o)
        : components::OComponent(std::move(o))
        , m_mem { std::move(o.m_mem) }
        , m_mem_strs { std::move(o.m_mem_strs) }
        , m_mem_sz { o.m_mem_sz }
        , m_name_sz { o.m_name_sz }
        , m_lay { o.m_lay }
        , m_cell_sz { o.m_cell_sz }
    {
    }
    inline Memory& operator=(Memory&& o)
    {
        components::OComponent::operator=(std::move(o));
        m_mem = std::move(o.m_mem);
        m_mem_strs = std::move(o.m_mem_strs);
        m_mem_sz = o.m_mem_sz;
        m_name_sz = o.m_name_sz;
        m_lay = o.m_lay;
        m_cell_sz = o.m_cell_sz;
        return *this;
    }
    inline virtual ~Memory() { };

    virtual void draw() override;
    virtual void update() override;
    virtual machine::actor::Actor poll(machine::Mctx) override;

    virtual auto mem() -> mem_t&;
    virtual auto mem() const -> const mem_t&;
    inline virtual auto set_layout(Layout l) -> void
    {
        m_lay = l;
        setup(*this);
    }
    inline virtual auto get_layout() const -> Layout { return m_lay; }

private:
    static std::pair<::Vector2, std::vector<std::string>>
    setup_mem(const mem_t&);
    static void setup(Memory&);
};
}
