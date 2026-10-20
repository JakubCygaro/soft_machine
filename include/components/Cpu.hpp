#pragma once
#include "common/Result.hpp"
#include "components/CpuInstPrelude.hpp"
#include "components/GameGraphElements.hpp"
#include "facelift/Fl.hpp"
#include "game/Styling.hpp"
#include "game/XmlMarshalling.hpp"
#include "machine/Actor.hpp"
#include "machine/MachineContext.hpp"
#include <cstring>
#include <memory>
#include <stdexcept>

namespace components {
class CPU : public components::OComponent {
public:
    struct Register {
        int idx { };
        static Result<std::runtime_error, Register> parse_xml(const char* str);
        std::string to_string() const noexcept;
    };
    struct Instruction : game::xml::MarshallToXml {
        virtual std::string to_string() = 0;
        virtual std::string to_render_string() = 0;
        virtual ~Instruction() = default;
    };
    static Result<std::runtime_error, Instruction*>
    instruction_from_xml(pugi::xml_node&);

public:
    inline static const ::Color INST_COLOR = ::BLACK;
    inline static const ::Color BODY_COLOR = ::LIME;
    inline static const ::Color MARKED_COLOR = ::BLUE;
    inline static constexpr const size_t REG_COUNT = 10;
    inline static constexpr const int REG_FONT_SIZE = 20;
    using code_t = std::vector<std::unique_ptr<Instruction>>;
    using regs_t = std::array<int, REG_COUNT>;

private:
    code_t m_code;
    regs_t m_regs { };
    struct inst_draw_data {
        std::string rep;
        ::Vector2 dims;
    };
    std::vector<inst_draw_data> m_inst_draw { };
    ::Vector2 m_max_inst_dims { };
    ::Vector2 m_name_sz;
    int m_pc { };
    ::Vector2 m_max_reg_dims { };
    struct reg_draw_data {
        std::string rep;
        ::Vector2 dims;
        bool marked;
    };
    std::array<reg_draw_data, REG_COUNT> m_reg_draw_data;
    std::pair<::Vector2, std::string> m_pc_draw_data;
    float m_inst_dims_y_acc;

public:
    CPU(std::string name, code_t&& code);
    CPU(fl::string_param_t name);
    CPU(const CPU&) = delete;
    CPU& operator=(const CPU&) = delete;
    CPU(CPU&& o);
    CPU& operator=(CPU&& o);
    virtual ~CPU();

    code_t& code();

    virtual void draw() override;
    virtual void update() override;
    virtual void set_styling(game::Style::shared_ptr_t s) override;

    virtual machine::actor::Actor<void> poll(machine::Mctx) override;

    virtual const char*
    marshall_to_xml_name() const noexcept override;
    virtual void
    marshall_to_xml(pugi::xml_node& self) const noexcept override;

private:
    static void setup(CPU& self);
    static void setup_regs(CPU& self);
    static void setup_pc(CPU& self);
    static void setup_bounds(CPU& self);
    void unmark_all(void);
    int& reg(const Register&);
    machine::actor::Actor<void> exec_wait(machine::Mctx&, const InstWait*);
    machine::actor::Actor<void> exec_load(machine::Mctx&, const InstLoad*);
    machine::actor::Actor<void> exec_if(machine::Mctx&, const InstIf*);

protected:
};
}
