#include "common/String.hpp"
#include "components/Cpu.hpp"
#include "components/CpuInst.hpp"
#include "imgui.h"
#include "imgui_stdlib.h"
#include <concepts>
#include <format>
#include <functional>
#include <map>
#include <memory>
namespace components {

template <std::derived_from<CPU::Instruction> T>
auto make_default_init() -> auto
{
    return []() -> CPUEditor::inst_ptr_t {
        return std::make_unique<T>();
    };
}

CPUEditor::CPUEditor(CPUEditorKey, CPU* cpu)
    : m_cpu { cpu }
    , m_instruction_map { }
{
    using namespace std::placeholders;
    m_instruction_map["wait"] = InstView {
        .init = make_default_init<InstWait>(),
        .edit_dispatch = [this](inst_t* i) -> bool {
            return this->edit_concrete_inst(dynamic_cast<InstWait*>(i));
        },
    };
    m_instruction_map["load"] = InstView {
        .init = make_default_init<InstLoad>(),
        .edit_dispatch = [this](inst_t* i) -> bool {
            return this->edit_concrete_inst(dynamic_cast<InstLoad*>(i));
        },
    };
    m_instruction_map["send"] = InstView {
        .init = make_default_init<InstSend>(),
        .edit_dispatch = [this](inst_t* i) -> bool {
            return this->edit_concrete_inst(dynamic_cast<InstSend*>(i));
        },
    };
    m_instruction_map["if"] = InstView {
        .init = make_default_init<InstIf>(),
        .edit_dispatch = [this](inst_t* i) -> bool {
            return this->edit_concrete_inst(dynamic_cast<InstIf*>(i));
        },
    };
    m_instruction_map["add"] = InstView {
        .init = make_default_init<InstAdd>(),
        .edit_dispatch = [this](inst_t* i) -> bool {
            return this->edit_concrete_inst(dynamic_cast<InstAdd*>(i));
        },
    };
    m_instruction_map["sub"] = InstView {
        .init = make_default_init<InstSub>(),
        .edit_dispatch = [this](inst_t* i) -> bool {
            return this->edit_concrete_inst(dynamic_cast<InstSub*>(i));
        },
    };
    m_instruction_map["movi"] = InstView {
        .init = make_default_init<InstMovi>(),
        .edit_dispatch = [this](inst_t* i) -> bool {
            return this->edit_concrete_inst(dynamic_cast<InstMovi*>(i));
        },
    };
}
bool CPUEditor::draw_update()
{
    auto& code = m_cpu->m_code;
    auto& regs = m_cpu->m_regs;
    bool flag = false;
    if (ImGui::CollapsingHeader("cpu")) {
        ImGui::SeparatorText("general");
        flag |= ImGui::Checkbox("Pause Execution", &m_cpu->m_execution_paused);
        if (ImGui::InputInt("Program Counter", &m_cpu->m_pc)) {
            m_cpu->m_pc = std::clamp(m_cpu->m_pc, 0, static_cast<int>(code.size() - 1));
            flag |= true;
        }
        if (ImGui::TreeNode("registers")) {
            for (auto r = 0UL; r < regs.size(); r++) {
                ImGui::PushID(r);
                flag |= ImGui::InputInt(std::format("R{}", r).c_str(), &regs[r]);
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("code")) {
            if (ImGui::BeginTable("Instructions", 3,
                    ImGuiTableFlags_Resizable
                        | ImGuiTableFlags_NoSavedSettings
                        | ImGuiTableFlags_Borders)) {
                for (auto i = 0UL; i < code.size(); i++) {
                    ImGui::PushID(&code[i]);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%02zu", i);
                    // ImGui::TableNextColumn();
                    if (i > 0) {
                        ImGui::SameLine();
                        if (ImGui::ArrowButton("Move Up", ImGuiDir_Up)) {
                            flag = true;
                            std::swap(code[i], code[i - 1]);
                        }
                    }
                    if (i < code.size() - 1) {
                        ImGui::SameLine();
                        if (ImGui::ArrowButton("Move Down", ImGuiDir_Down)) {
                            flag = true;
                            std::swap(code[i], code[i + 1]);
                        }
                    }
                    ImGui::TableNextColumn();
                    flag |= instruction_edit(i, code[i]);
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
    }
    return flag;
}
bool CPUEditor::instruction_edit(
    std::size_t idx,
    std::shared_ptr<CPU::Instruction>& inst)
{
    bool flag = false;
    using namespace common::str;
    auto inst_name = inst->get_name();
    if (ImGui::BeginCombo("Type", inst_name.data())) {
        int counter = 0;
        for (const auto& [k, v] : m_instruction_map) {
            ImGui::PushID(counter++);
            if (ImGui::Selectable(k.c_str())) {
                if (inst_name.compare(k) != 0) {
                    inst = v.init();
                    flag = true;
                }
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    ImGui::TableNextColumn();
    auto& inst2 = m_cpu->m_code.at(idx);
    inst_name = inst2->get_name();
    if (m_instruction_map.contains(inst_name)) {
        flag |= m_instruction_map[inst_name].edit_dispatch(inst.get());
    }
    return flag;
}
namespace {
    void clamp_reg(CPU::Register& r)
    {
        r.idx = std::clamp(r.idx, 0, static_cast<int>(CPU::REG_COUNT - 1));
    }
}
bool CPUEditor::edit_concrete_inst(InstWait* w)
{
    bool flag = false;
    flag |= ImGui::InputText("ON", &w->attrs->on);
    if (ImGui::InputInt("INTO", &w->attrs->into.idx)) {
        clamp_reg(w->attrs->into);
        flag = true;
    }
    return flag;
}
bool CPUEditor::edit_concrete_inst(InstLoad* l)
{
    bool flag = false;
    flag |= ImGui::InputText("FROM", &l->attrs->from);
    if (ImGui::InputInt("AT", &l->attrs->at)) {
        flag = true;
    }
    if (ImGui::InputInt("INTO", &l->attrs->into.idx)) {
        clamp_reg(l->attrs->into);
        flag = true;
    }
    return flag;
}
bool CPUEditor::edit_concrete_inst(InstAdd* l)
{
    bool flag = false;
    if (ImGui::InputInt("SRC", &l->attrs->src.idx)) {
        clamp_reg(l->attrs->src);
        flag = true;
    }
    if (ImGui::InputInt("DST", &l->attrs->dst.idx)) {
        clamp_reg(l->attrs->dst);
        flag = true;
    }
    return flag;
}
bool CPUEditor::edit_concrete_inst(InstSub* s)
{
    bool flag = false;
    if (ImGui::InputInt("SRC", &s->attrs->src.idx)) {
        clamp_reg(s->attrs->src);
        flag = true;
    }
    if (ImGui::InputInt("DST", &s->attrs->dst.idx)) {
        clamp_reg(s->attrs->dst);
        flag = true;
    }
    return flag;
}
bool CPUEditor::edit_concrete_inst(InstSend* s)
{
    bool flag = false;
    flag |= ImGui::InputText("TO", &s->attrs->to);
    if (ImGui::InputInt("FROM", &s->attrs->from.idx)) {
        clamp_reg(s->attrs->from);
        flag = true;
    }
    return flag;
}
bool CPUEditor::edit_concrete_inst(InstIf* f)
{
    struct op_rep {
        InstIf::Op op;
        std::string rep;
    };
    using namespace common::reflect;
    static std::vector<op_rep> operators = [] {
        std::vector<op_rep> ret;
        for (auto i = std::to_underlying(InstIf::Op::L);
            i <= std::to_underlying(InstIf::Op::GE);
            i++) {
            const auto en = static_cast<InstIf::Op>(i);
            ret.push_back({ en, common::reflect::enum_to_string(en) });
        }
        return ret;
    }();
    bool flag = false;
    if (ImGui::InputInt("A", &f->attrs->a.idx)) {
        clamp_reg(f->attrs->a);
        flag = true;
    }
    // TODO: something is no yes here
    if (ImGui::BeginCombo("OP",
            enum_to_string_view(f->attrs->op).data())) {
        int counter = 0;
        for (const auto& [op, rep] : operators) {
            ImGui::PushID(counter++);
            if ((ImGui::Selectable(rep.c_str()))) {
                f->attrs->op = op;
                flag = true;
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    if (ImGui::InputInt("B", &f->attrs->b.idx)) {
        clamp_reg(f->attrs->b);
        flag = true;
    }
    if (ImGui::InputInt("JMP", &f->attrs->jmpto)) {
        f->attrs->jmpto = std::clamp(f->attrs->jmpto,
            0,
            static_cast<int>(m_cpu->m_code.size() - 1));
        flag = true;
    }
    return flag;
}
bool CPUEditor::edit_concrete_inst(InstMovi* m)
{
    bool flag = false;
    if (ImGui::InputInt("INTO", &m->attrs->into.idx)) {
        clamp_reg(m->attrs->into);
        flag = true;
    }
    if (ImGui::InputInt("IVAL", &m->attrs->ival)) {
        flag = true;
    }
    return flag;
}
}
