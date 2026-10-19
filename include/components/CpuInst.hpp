#pragma once
#include "common/reflect/Enum.hpp"
#include "components/Cpu.hpp"
#include <string>

namespace components {
struct InstWait : public CPU::Instruction {
    struct wait_attr {
        std::string on;
        CPU::Register into;
    };
    game::xml::Attribute<wait_attr> attrs;
    std::string to_string() override;
    std::string to_render_string() override;
    const char* marshall_to_xml_name() const noexcept override;
    void marshall_to_xml(pugi::xml_node& self) const noexcept override;
    virtual ~InstWait();
};
struct InstLoad : public CPU::Instruction {
    struct load_attr {
        std::string from;
        int at;
        CPU::Register into;
    };
    game::xml::Attribute<load_attr> attrs;
    std::string to_string() override;
    std::string to_render_string() override;
    const char* marshall_to_xml_name()
        const noexcept override;
    void marshall_to_xml(pugi::xml_node& self)
        const noexcept override;
    virtual ~InstLoad();
};
struct InstSend : public CPU::Instruction {
    struct send_attr {
        CPU::Register from;
        std::string to;
    };
    game::xml::Attribute<send_attr> attrs;
    std::string to_string() override;
    std::string to_render_string() override;
    const char* marshall_to_xml_name()
        const noexcept override;
    void marshall_to_xml(pugi::xml_node& self)
        const noexcept override;
    virtual ~InstSend();
};
struct InstMovi : public CPU::Instruction {
    struct movi_attr {
        CPU::Register into;
        int ival;
    };
    virtual ~InstMovi();
    game::xml::Attribute<movi_attr> attrs;
    std::string to_string() override;
    std::string to_render_string() override;
    const char* marshall_to_xml_name()
        const noexcept override;
    void marshall_to_xml(pugi::xml_node& self)
        const noexcept override;
};
struct InstIf : public CPU::Instruction {
    // TODO: enum_to_string<E>
    enum class Op {
        L,
        LE,
        E,
        NE,
        G,
        GE
    };
    struct if_attr {
        CPU::Register a, b;
        Op op;
        int jmpto;
    };
    game::xml::Attribute<if_attr> attrs;
    virtual ~InstIf();
    std::string to_string() override;
    std::string to_render_string() override;
    const char* marshall_to_xml_name()
        const noexcept override;
    void marshall_to_xml(pugi::xml_node& self)
        const noexcept override;
};
struct arth_attr {
    CPU::Register src, dst;
};
struct InstAdd : public CPU::Instruction {
    game::xml::Attribute<arth_attr> attrs;
    virtual ~InstAdd();
    std::string to_string() override;
    std::string to_render_string() override;
    const char* marshall_to_xml_name()
        const noexcept override;
    void marshall_to_xml(pugi::xml_node& self)
        const noexcept override;
};
struct InstSub : public CPU::Instruction {
    game::xml::Attribute<arth_attr> attrs;
    virtual ~InstSub();
    std::string to_string() override;
    std::string to_render_string() override;
    const char* marshall_to_xml_name()
        const noexcept override;
    void marshall_to_xml(pugi::xml_node& self)
        const noexcept override;
};
}
