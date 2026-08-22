#pragma once
#include "common/Result.hpp"
#include "components/GameGraphElements.hpp"
#include "game/Xml.hpp"
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
        inline static Result<std::runtime_error, Register> parse(const char* str)
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
        inline virtual ~Instruction() { }
    };
    struct InstWait : public Instruction {
        std::string on;
        int into;
        inline virtual std::string to_string()
        {
            return std::format("WAIT ON {} INTO R{}", on, into);
        }
        inline virtual ~InstWait() { }
    };
    struct InstLoad : public Instruction {
        std::string from;
        int at;
        int into;
        inline virtual std::string to_string()
        {
            return std::format("LOAD FROM {} AT {} INTO R{}", from, at, into);
        }
        inline virtual ~InstLoad() { }
    };
    struct InstSend : public Instruction {
        int from;
        std::string to;
        inline virtual std::string to_string()
        {
            return std::format("SEND FROM R{} TO {}", from, to);
        }
        inline virtual ~InstSend() { }
    };
    struct InstAdd : public Instruction {
        int src, dst;
        inline virtual std::string to_string()
        {
            return std::format("ADD R{} INTO R{}", src, dst);
        }
        inline virtual ~InstAdd() { }
    };
    static Result<std::runtime_error, Instruction*>
    instruction_from_xml(pugi::xml_node&);

public:
    inline static constexpr const size_t REG_COUNT = 10;
    using code_t = std::vector<std::unique_ptr<Instruction>>;
    using regs_t = std::array<int, REG_COUNT>;

private:
    code_t m_code;
    regs_t m_regs { };

public:
    inline CPU(
        std::string name,
        code_t&& code)
        : components::OComponent(name)
        , m_code { std::move(code) }
    {
    }
    CPU(const CPU&) = delete;
    CPU& operator=(const CPU&) = delete;
    inline CPU(CPU&& o)
        : components::OComponent(std::move(o))
        , m_code { std::move(o.m_code) }
        , m_regs { std::move(o.m_regs) }
    {
    }
    inline CPU& operator=(CPU&& o)
    {
        components::OComponent::operator=(std::move(o));
        m_code = std::move(o.m_code);
        m_regs = std::move(o.m_regs);
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
};
}
