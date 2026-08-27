#pragma once
#include "common/Result.hpp"
#include "components/GameGraphElements.hpp"
#include "game/Xml.hpp"
#include "game/XmlMarshalling.hpp"
#include "machine/Actor.hpp"
#include "machine/MachineContext.hpp"
#include <cstring>
#include <format>
#include <memory>
#include <stdexcept>

namespace components {
class CPU : public components::OComponent {
public:
    struct Register {
        int idx { };
        inline static Result<std::runtime_error, Register> parse_xml(const char* str)
        {
            if (std::strlen(str) != 2
                || (str[0] != 'R' && str[0] != 'r')
                || !std::isdigit(str[1])
                || (str[1] - '0') >= static_cast<int>(CPU::REG_COUNT)) {
                return {
                    std::runtime_error(std::format("{} not a valid register", str))
                };
            }
            return {
                Register { str[1] - '0' }
            };
        };
    };
    struct Instruction {
        virtual std::string to_string() = 0;
        virtual std::string to_render_string() = 0;
        inline virtual ~Instruction() { }
    };
    struct InstWait : public Instruction {
        struct wait_attr {
            std::string on;
            Register into;
        };
        game::xml::Attribute<wait_attr> attrs;
        inline virtual std::string to_string()
        {
            return std::format("WAIT ON {} INTO R{}", attrs->on, attrs->into.idx);
        }
        inline virtual std::string to_render_string()
        {
            return std::format("WAIT\n\tON {}\n\tINTO R{}", attrs->on, attrs->into.idx);
        }
        inline virtual ~InstWait() { }
    };
    struct InstLoad : public Instruction {
        struct load_attr {
            std::string from;
            int at;
            Register into;
        };
        game::xml::Attribute<load_attr> attrs;
        inline virtual std::string to_string()
        {
            return std::format("LOAD FROM {} AT {} INTO R{}",
                attrs->from, attrs->at, attrs->into.idx);
        }
        inline virtual std::string to_render_string()
        {
            return std::format("LOAD\n\tFROM {}\n\tAT {}\n\tINTO R{}",
                attrs->from, attrs->at, attrs->into.idx);
        }
        inline virtual ~InstLoad() { }
    };
    struct InstSend : public Instruction {
        struct send_attr {
            Register from;
            std::string to;
        };
        game::xml::Attribute<send_attr> attrs;
        inline virtual std::string to_string()
        {
            return std::format("SEND FROM R{} TO {}",
                attrs->from.idx, attrs->to);
        }
        inline virtual std::string to_render_string()
        {
            return std::format("SEND\n\tFROM R{}\n\tTO {}",
                attrs->from.idx, attrs->to);
        }
        inline virtual ~InstSend() { }
    };
    struct arth_attr {
        Register src, dst;
    };
    struct InstAdd : public Instruction {
        game::xml::Attribute<arth_attr> attr;
        inline virtual std::string to_string()
        {
            return std::format("ADD R{} INTO R{}",
                attr->src.idx, attr->dst.idx);
        }
        inline virtual std::string to_render_string()
        {
            return std::format("ADD R{} INTO R{}",
                attr->src.idx, attr->dst.idx);
        }
        inline virtual ~InstAdd() { }
    };
    struct InstSub : public Instruction {
        game::xml::Attribute<arth_attr> attr;
        inline virtual std::string to_string()
        {
            return std::format("SUB R{} INTO R{}",
                attr->src.idx, attr->dst.idx);
        }
        inline virtual std::string to_render_string()
        {
            return std::format("SUB R{} INTO R{}",
                attr->src.idx, attr->dst.idx);
        }
        inline virtual ~InstSub() { }
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
    ::Vector2 m_max_reg_dims {};
    struct reg_draw_data {
        std::string rep;
        ::Vector2 dims;
        bool marked;
    };
    std::array<reg_draw_data, REG_COUNT> m_reg_draw_data;
    std::pair<::Vector2, std::string> m_pc_draw_data;

public:
    inline CPU(
        std::string name,
        code_t&& code)
        : components::OComponent(name)
        , m_code { std::move(code) }
    {
        setup(*this);
    }
    CPU(const CPU&) = delete;
    CPU& operator=(const CPU&) = delete;
    inline CPU(CPU&& o)
        : components::OComponent(std::move(o))
        , m_code { std::move(o.m_code) }
        , m_regs { std::move(o.m_regs) }
        , m_inst_draw { std::move(o.m_inst_draw) }
        , m_max_inst_dims { std::move(o.m_max_inst_dims) }
        , m_name_sz { std::move(o.m_name_sz) }
        , m_pc { std::move(o.m_pc) }
        , m_max_reg_dims { std::move(o.m_max_reg_dims) }
        , m_reg_draw_data { std::move(o.m_reg_draw_data) }
        , m_pc_draw_data { std::move(o.m_pc_draw_data) }
    {
    }
    inline CPU& operator=(CPU&& o)
    {
        components::OComponent::operator=(std::move(o));
        m_code = std::move(o.m_code);
        m_regs = std::move(o.m_regs);
        m_inst_draw = std::move(o.m_inst_draw);
        m_max_inst_dims = std::move(o.m_max_inst_dims);
        m_name_sz = std::move(o.m_name_sz);
        m_pc = std::move(o.m_pc);
        m_reg_draw_data = std::move(o.m_reg_draw_data);
        m_pc_draw_data = std::move(o.m_pc_draw_data);
        return *this;
    }
    inline virtual ~CPU() { };

    inline code_t& code()
    {
        return m_code;
    }

    virtual void draw() override;
    virtual void update() override;

    virtual machine::actor::Actor poll(machine::Mctx) override;

private:
    static void setup(CPU& self);
    static void setup_regs(CPU& self);
    static void setup_pc(CPU& self);
    static void setup_bounds(CPU& self);
    void unmark_all(void);
    int& reg(const Register&);
};
}
