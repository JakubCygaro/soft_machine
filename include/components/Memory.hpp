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
    const Color BODY_COLOR = ::BLUE;
    const Color DATA_COLOR = ::BLACK;
    const Color MEM_CELL_COLOR = ::WHITE;

public:
    using mem_t = std::vector<int>;

protected:
    mem_t m_mem;
    std::string m_mem_str;
    ::Vector2 m_mem_sz;
    ::Vector2 m_name_sz;

public:
    inline Memory(
        std::string name,
        mem_t&& mem)
        : components::OComponent(name)
        , m_mem(std::move(mem))
    {
        // using namespace game::resources;
        // auto [dims, txt] = setup_mem(m_mem);
        // constexpr const auto def = game::resources::get_default_node_size();
        // m_mem_sz = dims;
        // m_name_sz = ::MeasureTextEx(
        //     get_node_font(),
        //     name.c_str(),
        //     default_node_font_size(),
        //     default_font_spacing());
        // m_bounds.width = std::max(def.x + 40.0f + m_name_sz.x, dims.x);
        // m_bounds.height = std::max(def.y + 40.0f + m_name_sz.y, dims.y);
        // m_mem_str = txt;
        setup(*this);
    }
    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;
    inline Memory(Memory&& o)
        : components::OComponent(std::move(o))
        , m_mem { std::move(o.m_mem) }
        , m_mem_str { std::move(o.m_mem_str) }
        , m_mem_sz { o.m_mem_sz }
        , m_name_sz { o.m_name_sz }
    {
    }
    inline Memory& operator=(Memory&& o)
    {
        components::OComponent::operator=(std::move(o));
        m_mem = std::move(o.m_mem);
        m_mem_str = std::move(o.m_mem_str);
        m_mem_sz = o.m_mem_sz;
        m_name_sz = o.m_name_sz;
        return *this;
    }
    inline virtual ~Memory() { };

    virtual void draw() override;
    virtual void update() override;
    virtual machine::actor::Actor poll(machine::Mctx) override;

    virtual auto mem() -> mem_t&;
    virtual auto mem() const -> const mem_t&;

private:
    static std::pair<::Vector2, std::string> setup_mem(const mem_t&);
    static void setup(Memory&);
};
}
