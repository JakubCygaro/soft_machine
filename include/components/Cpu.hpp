#pragma once
#include "common/Result.hpp"
#include "components/CpuInstPrelude.hpp"
#include "components/GameGraphElements.hpp"
#include "facelift/Fl.hpp"
#include "game/Editable.hpp"
#include "game/Styling.hpp"
#include "game/XmlMarshalling.hpp"
#include "machine/Actor.hpp"
#include "machine/MachineContext.hpp"
#include <cstring>
#include <expected>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

namespace components {
class CPU;
struct CPUEditorKey {

private:
    CPUEditorKey() = default;
    friend class CPU;

public:
};

class CPU : public components::OComponent {
public:
    friend struct CPUEditor;
    struct Register {
        int idx { };
        static Result<std::runtime_error, Register> parse_xml(const char* str);
        std::string to_string() const noexcept;
        constexpr auto operator<=>(const Register&) const = default;
    };
    struct Instruction : game::xml::MarshallToXml {
        virtual const std::string_view get_name() const = 0;
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
    // shared pointers because the editor can modify the code mid execution
    using code_t = std::vector<std::shared_ptr<Instruction>>;
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

    virtual std::unique_ptr<game::Editor> get_editor_object() override;
    virtual void notify_after_styling() override;

private:
    static void setup(CPU& self);
    static void setup_regs(CPU& self);
    static void setup_pc(CPU& self);
    static void setup_bounds(CPU& self);
    void unmark_all(void);
    int& reg(const Register&);
    machine::actor::Actor<void> exec_wait(machine::Mctx&, const InstWait*);
    machine::actor::Actor<void> exec_load(machine::Mctx&, const InstLoad*);
    void exec_if(machine::Mctx&, const InstIf*);

protected:
};
namespace cpu {
    using Iden = std::string;
    using Reg = CPU::Register;
    using Eof = struct { };
    using Token = std::variant<
        Iden,
        std::int64_t,
        Reg,
        Eof>;
    std::string stringify(const Token&);
    class Tokenizer {
        std::istringstream m_iss;

    public:
        Tokenizer(std::istringstream&&);
        Tokenizer(Tokenizer&&) = default;
        Tokenizer& operator=(Tokenizer&&) = default;
        Tokenizer(const Tokenizer&) = delete;
        Tokenizer& operator=(const Tokenizer&) = delete;
        std::expected<Token, std::string> read_next(void);
    };
    class Parser {
        Tokenizer m_tok;

    public:
        Parser(std::istringstream&&);
        Parser(Parser&&) = default;
        Parser& operator=(Parser&&) = default;
        Parser(const Parser&) = delete;
        Parser& operator=(const Parser&) = delete;
        std::expected<std::unique_ptr<CPU::Instruction>, std::string>
        read_next(void);

    private:
        std::expected<std::unique_ptr<CPU::Instruction>, std::string>
        read_instruction(Iden start);

        template <class T, class Inspect = std::optional<T>>
        inline std::expected<T, std::string> expect_next(Inspect inspect = std::nullopt)
        {
            auto t = m_tok.read_next();
            if (!t.has_value())
                return std::unexpected<std::string>(t.error());
            auto t_val = std::move(t.value());
            if (!std::holds_alternative<T>(t_val))
                return std::unexpected<std::string>(
                    std::format("Unexpected token '{}'", stringify(t_val)));
            if constexpr (std::is_same_v<std::optional<T>, Inspect>) {
                if (!inspect)
                    return std::get<T>(t_val);
                if (std::get<T>(t_val) != *inspect) {
                    return std::unexpected<std::string>(
                        std::format("Token value mismatch '{}' <> '{}'",
                            stringify(t_val), stringify(*inspect)));
                } else {
                    return std::get<T>(t_val);
                }
            } else if constexpr (std::is_same_v<T, Inspect>
                || std::is_same_v<T, std::nullopt_t>) {
                if (std::get<T>(t_val) != inspect) {
                    return std::unexpected<std::string>(
                        std::format("Token value mismatch '{}' <> '{}'",
                            stringify(t_val), inspect));
                } else {
                    return std::get<T>(t_val);
                }
            } else if constexpr (std::is_invocable_v<Inspect>) {
                if (!Inspect(std::get<T>(t_val))) {
                    return std::unexpected<std::string>(
                        std::format("Bad token value '{}'",
                            stringify(t_val)));
                } else {
                    return std::get<T>(t_val);
                }
            } else
                static_assert(false, "expect_next bad template Inspect parameter type");
        }
    };

}
struct CPUEditor : public game::Editor {
    using inst_t = CPU::Instruction;
    using inst_ptr_t = std::shared_ptr<CPU::Instruction>;

private:
    using default_init_fn = std::function<inst_ptr_t()>;
    using edit_fn = std::function<bool(inst_t*)>;
    struct InstView {
        default_init_fn init;
        edit_fn edit_dispatch;
    };
    CPU* m_cpu { nullptr };
    std::map<std::string, InstView, std::less<>> m_instruction_map;

public:
    virtual ~CPUEditor() = default;
    CPUEditor(CPUEditorKey, CPU*);
    CPUEditor(const CPUEditor&) = delete;
    CPUEditor& operator=(const CPUEditor&) = delete;
    CPUEditor(CPUEditor&&) = default;
    CPUEditor& operator=(CPUEditor&&) = default;
    virtual bool draw_update() override;

private:
    bool instruction_edit(
        std::size_t idx,
        std::shared_ptr<CPU::Instruction>& inst);
    bool edit_concrete_inst(InstWait*);
    bool edit_concrete_inst(InstLoad*);
    bool edit_concrete_inst(InstSend*);
    bool edit_concrete_inst(InstIf*);
    bool edit_concrete_inst(InstAdd*);
    bool edit_concrete_inst(InstSub*);
};
}
