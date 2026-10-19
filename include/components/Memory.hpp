#pragma once
#include "components/GameGraphElements.hpp"
#include "components/msg/Message.hpp"
#include "facelift/Fl.hpp"
#include "game/Styling.hpp"
#include "machine/Actor.hpp"
#include "machine/MachineContext.hpp"
#include <raylib.h>
#include <string>

namespace components {
namespace mem {
    struct MemoryWrite : public components::msg::Msg {
        size_t at;
        int val;
        MemoryWrite() = delete;
        MemoryWrite(size_t at, int val);
        virtual std::string type_name() const override;
        virtual std::string to_display_string() const override;
    };
    struct MemoryRead : public components::msg::Msg {
        size_t at;
        MemoryRead() = delete;
        MemoryRead(size_t at);
        virtual std::string type_name() const override;
        virtual std::string to_display_string() const override;
    };
    struct MemoryResponse : public components::msg::Msg {
        int val;
        MemoryResponse() = delete;
        MemoryResponse(int val);
        virtual std::string type_name() const override;
        virtual std::string to_display_string() const override;
    };
    struct MemoryFail : public components::msg::Msg {
        size_t at;
        MemoryFail() = delete;
        MemoryFail(size_t at);
        virtual std::string type_name() const override;
        virtual std::string to_display_string() const override;
    };
}
class Memory : public components::OComponent {
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
    Memory(std::string name, mem_t&& mem);
    Memory(fl::string_param_t name);
    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;
    Memory(Memory&& o);
    Memory& operator=(Memory&& o);
    virtual ~Memory();

    virtual void draw() override;
    virtual void update() override;
    virtual machine::actor::Actor<void> poll(machine::Mctx) override;

    virtual auto mem() -> mem_t&;
    virtual auto mem() const -> const mem_t&;
    virtual auto set_layout(Layout l) -> void;
    virtual auto get_layout() const -> Layout;

    virtual const char*
    marshall_to_xml_name() const noexcept override;
    virtual void
    marshall_to_xml(pugi::xml_node&) const noexcept override;

    static Result<std::runtime_error, mem_t>
    parse_memset_from_text(const std::string& txt) noexcept;

    virtual void set_size(const ::Vector2& sz) override;
    virtual void set_styling(game::Style::shared_ptr_t s) override;

private:
    static std::pair<::Vector2, std::vector<Memory::cell_str_dim>>
    setup_mem(Memory& self, const mem_t&);
    static void setup(Memory&);
};
}
