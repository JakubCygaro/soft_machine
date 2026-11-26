#include "common/String.hpp"
#include "components/Cpu.hpp"
#include "components/CpuInst.hpp"
#include <cctype>
#include <cstdint>
#include <format>
#include <memory>
#include <string>
#include <type_traits>
#include <variant>

namespace components::cpu {
Tokenizer::Tokenizer(std::istringstream&& iss)
    : m_iss { std::move(iss) }
{
}
std::expected<Token, std::string>
Tokenizer::read_next(void)
{
    char ch { };
    while (1) {
        m_iss.get(ch);
        if (m_iss.eof())
            return Eof { };
        if (!std::isspace(ch)) {
            break;
        }
    }
    std::expected<Token, std::string> t;
    if (std::isdigit(ch)) {
        m_iss.unget();
        std::int64_t d;
        m_iss >> d;
        if (m_iss.failbit) {
            t = "failed to read 64-bit signed integer";
        }
        t = d;
    } else if (std::isalnum(ch)) {
        if (ch == 'r' || ch == 'R') {
            unsigned char r;
            m_iss >> r;
            if (m_iss.failbit) {
                t = "failed to read register number";
            }
            if (r >= CPU::REG_COUNT) {
                t = std::format("no such register '{}{}'", ch, r);
            } else {
                t = CPU::Register { r };
            }
        } else {
            m_iss.unget();
            std::string i;
            m_iss >> i;
            t = std::move(i);
        }
    }
    return t;
}
Parser::Parser(std::istringstream&& iss)
    : m_tok { std::move(iss) }
{
}
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
std::string stringify(const Token& t)
{
    return std::visit(
        overloaded {
            [=](const CPU::Register& r) { return std::format("R{}", r.idx); },
            [=](const std::int64_t& d) { return std::to_string(d); },
            [=](const Iden& i) { return i; },
            [=](const Eof&) { return std::string("<EOF>"); } },
        t);
}
std::expected<std::unique_ptr<CPU::Instruction>, std::string>
Parser::read_instruction(Iden start)
{
    using ret_t = std::expected<std::unique_ptr<CPU::Instruction>, std::string>;
    using inst_t = CPU::Instruction;
    using inst_ptr = std::shared_ptr<CPU::Instruction>;
    using namespace common::str;
    if (equal(start, "wait")) {
        auto* wait = new InstWait { };
        return expect_next<Iden>(Iden { "on" })
            .and_then([&](auto) {
                return expect_next<Iden>();
            })
            .and_then([&](Iden port) {
                wait->attrs->on = port;
                return expect_next<Iden>(Iden { "into" });
            })
            .and_then([&](auto) {
                return expect_next<Reg>();
            })
            .and_then([=](Reg into) {
                wait->attrs->into = into;
                return ret_t(wait);
            });
    }
    return std::unexpected(
        std::format("Unknown instruction starting with '{}'", start));
}
std::expected<std::unique_ptr<CPU::Instruction>, std::string>
Parser::read_next(void)
{
    auto next = m_tok.read_next();
    if (!next.has_value())
        return std::unexpected(next.error());
    const auto t = *next;
    if (std::holds_alternative<Eof>(t))
        return nullptr;
    if (std::holds_alternative<Iden>(t)) {
        return read_instruction(std::get<Iden>(t));
    }
    return std::unexpected(std::format(
        "Unexpected token in input '{}'", stringify(t)));
}
}
