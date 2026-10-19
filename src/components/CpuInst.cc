#include "components/CpuInst.hpp"
namespace components {
InstWait::~InstWait() { }
std::string InstWait::to_string()
{
    return std::format("WAIT ON {} INTO R{}", attrs->on, attrs->into.idx);
}
std::string InstWait::to_render_string()
{
    return std::format("WAIT\n\tON {}\n\tINTO R{}", attrs->on, attrs->into.idx);
}
const char* InstWait::marshall_to_xml_name()
    const noexcept
{
    return "wait";
}
void InstWait::marshall_to_xml(pugi::xml_node& self)
    const noexcept
{
    self.append_attribute("on")
        .set_value(attrs->on);
    self.append_attribute("into")
        .set_value(attrs->into.to_string());
}
InstLoad::~InstLoad() { }
std::string InstLoad::to_string()
{
    return std::format("LOAD FROM {} AT {} INTO R{}",
        attrs->from, attrs->at, attrs->into.idx);
}
std::string InstLoad::to_render_string()
{
    return std::format("LOAD\n\tFROM {}\n\tAT {}\n\tINTO R{}",
        attrs->from, attrs->at, attrs->into.idx);
}
const char* InstLoad::marshall_to_xml_name()
    const noexcept
{
    return "load";
}
void InstLoad::marshall_to_xml(pugi::xml_node& self)
    const noexcept
{
    self.append_attribute("from")
        .set_value(attrs->from);
    self.append_attribute("at")
        .set_value(attrs->at);
    self.append_attribute("into")
        .set_value(attrs->into.to_string());
}
InstSend::~InstSend() { }
std::string InstSend::to_string()
{
    return std::format("SEND FROM R{} TO {}",
        attrs->from.idx, attrs->to);
}
std::string InstSend::to_render_string()
{
    return std::format("SEND\n\tFROM R{}\n\tTO {}",
        attrs->from.idx, attrs->to);
}
const char* InstSend::marshall_to_xml_name()
    const noexcept
{
    return "send";
}
void InstSend::marshall_to_xml(pugi::xml_node& self)
    const noexcept
{
    self.append_attribute("from")
        .set_value(attrs->from.to_string());
    self.append_attribute("to")
        .set_value(attrs->to);
}
InstMovi::~InstMovi() { }
std::string InstMovi::to_string()
{
    return std::format("MOVI {} INTO R{}",
        attrs->ival, attrs->into.idx);
}
std::string InstMovi::to_render_string()
{
    return to_string();
}
const char* InstMovi::marshall_to_xml_name()
    const noexcept
{
    return "movi";
}
void InstMovi::marshall_to_xml(pugi::xml_node& self)
    const noexcept
{
    self.append_attribute("into")
        .set_value(attrs->into.to_string());
    self.append_attribute("ival")
        .set_value(attrs->ival);
}
InstIf::~InstIf() { }
std::string InstIf::to_string()
{
    return std::format("IF R{} {} R{} JMPTO {}",
        attrs->a.idx,
        static_cast<int>(attrs->op),
        attrs->b.idx,
        attrs->jmpto);
}
std::string InstIf::to_render_string()
{
    return std::format("IF R{} {} R{}\n\tJMPTO {}",
        attrs->a.idx,
        common::reflect::enum_to_string<Op>(attrs->op),
        attrs->b.idx,
        attrs->jmpto);
}
const char* InstIf::marshall_to_xml_name()
    const noexcept
{
    return "if";
}
void InstIf::marshall_to_xml(pugi::xml_node& self)
    const noexcept
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
InstAdd::~InstAdd() { }
std::string InstAdd::to_string()
{
    return std::format("ADD R{} INTO R{}",
        attrs->src.idx, attrs->dst.idx);
}
std::string InstAdd::to_render_string()
{
    return std::format("ADD R{} INTO R{}",
        attrs->src.idx, attrs->dst.idx);
}
const char* InstAdd::marshall_to_xml_name()
    const noexcept
{
    return "add";
}
void InstAdd::marshall_to_xml(pugi::xml_node& self)
    const noexcept
{
    self.append_attribute("src")
        .set_value(attrs->src.to_string());
    self.append_attribute("dst")
        .set_value(attrs->dst.to_string());
}
InstSub::~InstSub() { }
std::string InstSub::to_string()
{
    return std::format("SUB R{} INTO R{}",
        attrs->src.idx, attrs->dst.idx);
}
std::string InstSub::to_render_string()
{
    return std::format("SUB R{} INTO R{}",
        attrs->src.idx, attrs->dst.idx);
}
const char* InstSub::marshall_to_xml_name()
    const noexcept
{
    return "sub";
}
void InstSub::marshall_to_xml(pugi::xml_node& self)
    const noexcept
{
    self.append_attribute("src")
        .set_value(attrs->src.to_string());
    self.append_attribute("dst")
        .set_value(attrs->dst.to_string());
}
}
