#pragma once

#include "components/GameGraphElements.hpp"
#include "game/Xml.hpp"
#include "machine/Actor.hpp"
#include "machine/MachineContext.hpp"
#include <memory>
namespace components {
class CPU : public components::OComponent {
public:
    struct Instruction {
        inline virtual ~Instruction() { }
    };
    struct InstWait : public Instruction {
        std::string on;
        int into;
        inline virtual ~InstWait() { }
    };
    struct InstLoad : public Instruction {
        std::string from;
        int at;
        int into;
        inline virtual ~InstLoad() { }
    };
    struct InstSend : public Instruction {
        int from;
        std::string to;
        inline virtual ~InstSend() { }
    };
    struct InstAdd : public Instruction {
        int src, dst;
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
