#pragma once
#include "common/String.hpp"
#include "components/GameGraphElements.hpp"
#include "components/msg/Message.hpp"
#include "facelift/Fl.hpp"
#include "machine/Actor.hpp"
#include "machine/MachineContext.hpp"
#include <array>
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
class ANNOTATE(fl::component) Memory : public components::OComponent {
public:
    const Color BODY_COLOR = ::BLUE;
    const Color DATA_COLOR = ::BLACK;
    const Color MEM_CELL_COLOR = ::WHITE;
    const Color EMPTY_CELL = ::GRAY;
    inline const static float MARGIN = 40.0f;
    enum class Layout {
        Square,
        Vertical,
        Horizontal,
    };

public:
    using mem_t = std::vector<int>;

protected:
    using cell_str_dim = std::pair<std::string, ::Vector2>;
    mem_t m_mem;
    std::vector<cell_str_dim> m_mem_strs;
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
    ANNOTATE(fl::use_ctor)
    inline Memory(
        fl::string_param_t name)
        : components::OComponent(std::string(name.data(), name.size()))
        , m_mem({ 1, 2, 3, 4 })
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

    virtual const char*
    marshall_to_xml_name() const noexcept override;
    virtual void
    marshall_to_xml(pugi::xml_node&) const noexcept override;

    static Result<std::runtime_error, mem_t>
    parse_memset_from_text(const std::string& txt) noexcept
    {
        const auto parse_int =
            [](std::string& slice) -> Result<std::runtime_error, int> {
            int out;
            auto trimmed = common::trim(slice);
            auto res = std::from_chars(
                trimmed.data(),
                trimmed.data() + trimmed.size(),
                out,
                10);
            if (res.ec == std::errc::invalid_argument) {
                return { std::runtime_error(
                    std::format("failed to parse '{}' as integer", slice)) };
            }
            return { out };
        };
        auto s = txt.c_str();
        if (!s)
            return { std::runtime_error("empty memset node") };
        components::Memory::mem_t ret { };
        auto str = common::trim(std::string(s));
        str.erase(std::remove_if(
                      str.begin(),
                      str.end(),
                      [](auto c) {
                          return std::isspace(c) && c != ' ';
                      }),
            str.end());
        size_t pos = str.find(' ');
        size_t init_pos = 0;

        while (pos != std::string::npos) {
            auto i = common::trim(str.substr(init_pos, pos - init_pos));
            if (!i.empty()) {
                auto res = parse_int(i);
                if (res.iserr())
                    return { res.unwrap_err() };
                ret.push_back(std::get<int>(res));
            }
            init_pos = pos + 1;
            pos = str.find(' ', init_pos);
        }
        auto i = common::trim(str.substr(
            init_pos,
            std::min(
                pos,
                str.size())
                - init_pos + 1));
        if (!i.empty()) {
            auto res = parse_int(i);
            if (res.iserr())
                return { res.unwrap_err() };
            ret.push_back(std::get<int>(res));
        }
        return { ret };
    }

private:
    static std::pair<::Vector2, std::vector<Memory::cell_str_dim>>
    setup_mem(const mem_t&);
    static void setup(Memory&);
};
}
