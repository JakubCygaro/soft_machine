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
    inline virtual std::string to_string() override
    {
        return std::format("WAIT ON {} INTO R{}", attrs->on, attrs->into.idx);
    }
    inline virtual std::string to_render_string() override
    {
        return std::format("WAIT\n\tON {}\n\tINTO R{}", attrs->on, attrs->into.idx);
    }
    inline virtual const char* marshall_to_xml_name()
        const noexcept override
    {
        return "wait";
    }
    inline virtual void marshall_to_xml(pugi::xml_node& self)
        const noexcept override
    {
        self.append_attribute("on")
            .set_value(attrs->on);
        self.append_attribute("into")
            .set_value(attrs->into.to_string());
    }
    inline virtual ~InstWait() { }
};
struct InstLoad : public CPU::Instruction {
    struct load_attr {
        std::string from;
        int at;
        CPU::Register into;
    };
    game::xml::Attribute<load_attr> attrs;
    inline virtual std::string to_string() override
    {
        return std::format("LOAD FROM {} AT {} INTO R{}",
            attrs->from, attrs->at, attrs->into.idx);
    }
    inline virtual std::string to_render_string() override
    {
        return std::format("LOAD\n\tFROM {}\n\tAT {}\n\tINTO R{}",
            attrs->from, attrs->at, attrs->into.idx);
    }
    inline virtual const char* marshall_to_xml_name()
        const noexcept override
    {
        return "load";
    }
    inline virtual void marshall_to_xml(pugi::xml_node& self)
        const noexcept override
    {
        self.append_attribute("from")
            .set_value(attrs->from);
        self.append_attribute("at")
            .set_value(attrs->at);
        self.append_attribute("into")
            .set_value(attrs->into.to_string());
    }
    inline virtual ~InstLoad() { }
};
struct InstSend : public CPU::Instruction {
    struct send_attr {
        CPU::Register from;
        std::string to;
    };
    game::xml::Attribute<send_attr> attrs;
    inline virtual std::string to_string() override
    {
        return std::format("SEND FROM R{} TO {}",
            attrs->from.idx, attrs->to);
    }
    inline virtual std::string to_render_string() override
    {
        return std::format("SEND\n\tFROM R{}\n\tTO {}",
            attrs->from.idx, attrs->to);
    }
    inline virtual const char* marshall_to_xml_name()
        const noexcept override
    {
        return "send";
    }
    inline virtual void marshall_to_xml(pugi::xml_node& self)
        const noexcept override
    {
        self.append_attribute("from")
            .set_value(attrs->from.to_string());
        self.append_attribute("to")
            .set_value(attrs->to);
    }
    inline virtual ~InstSend() { }
};
struct InstMovi : public CPU::Instruction {
    struct movi_attr {
        CPU::Register into;
        int ival;
    };
    game::xml::Attribute<movi_attr> attrs;
    inline virtual std::string to_string() override
    {
        return std::format("MOVI {} INTO R{}",
            attrs->ival, attrs->into.idx);
    }
    inline virtual std::string to_render_string() override
    {
        return to_string();
    }
    inline virtual const char* marshall_to_xml_name()
        const noexcept override
    {
        return "movi";
    }
    inline virtual void marshall_to_xml(pugi::xml_node& self)
        const noexcept override
    {
        self.append_attribute("into")
            .set_value(attrs->into.to_string());
        self.append_attribute("ival")
            .set_value(attrs->ival);
    }
    inline virtual ~InstMovi() { }
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
    inline virtual std::string to_string() override
    {
        return std::format("IF R{} {} R{} JMPTO {}",
            attrs->a.idx,
            static_cast<int>(attrs->op),
            attrs->b.idx,
            attrs->jmpto);
    }
    inline virtual std::string to_render_string() override
    {
        return std::format("IF R{} {} R{}\n\tJMPTO {}",
            attrs->a.idx,
            common::reflect::enum_to_string<Op>(attrs->op),
            attrs->b.idx,
            attrs->jmpto);
    }
    inline virtual const char* marshall_to_xml_name()
        const noexcept override
    {
        return "if";
    }
    inline virtual void marshall_to_xml(pugi::xml_node& self)
        const noexcept override
    {
        self.append_attribute("a")
            .set_value(attrs->a.to_string());
        self.append_attribute("op")
            .set_value(common::reflect::enum_to_string(attrs->op));
        self.append_attribute("b")
            .set_value(attrs->b.to_string());
        self.append_attribute("jmpto")
            .set_value(attrs->jmpto);
    }
    inline virtual ~InstIf() { }
};
struct arth_attr {
    CPU::Register src, dst;
};
struct InstAdd : public CPU::Instruction {
    game::xml::Attribute<arth_attr> attrs;
    inline virtual std::string to_string() override
    {
        return std::format("ADD R{} INTO R{}",
            attrs->src.idx, attrs->dst.idx);
    }
    inline virtual std::string to_render_string() override
    {
        return std::format("ADD R{} INTO R{}",
            attrs->src.idx, attrs->dst.idx);
    }
    inline virtual const char* marshall_to_xml_name()
        const noexcept override
    {
        return "add";
    }
    inline virtual void marshall_to_xml(pugi::xml_node& self)
        const noexcept override
    {
        self.append_attribute("src")
            .set_value(attrs->src.to_string());
        self.append_attribute("dst")
            .set_value(attrs->dst.to_string());
    }
    inline virtual ~InstAdd() { }
};
struct InstSub : public CPU::Instruction {
    game::xml::Attribute<arth_attr> attrs;
    inline virtual std::string to_string() override
    {
        return std::format("SUB R{} INTO R{}",
            attrs->src.idx, attrs->dst.idx);
    }
    inline virtual std::string to_render_string() override
    {
        return std::format("SUB R{} INTO R{}",
            attrs->src.idx, attrs->dst.idx);
    }
    inline virtual const char* marshall_to_xml_name()
        const noexcept override
    {
        return "sub";
    }
    inline virtual void marshall_to_xml(pugi::xml_node& self)
        const noexcept override
    {
        self.append_attribute("src")
            .set_value(attrs->src.to_string());
        self.append_attribute("dst")
            .set_value(attrs->dst.to_string());
    }
    inline virtual ~InstSub() { }
};
}
