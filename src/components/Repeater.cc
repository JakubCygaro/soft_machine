#include "components/Repeater.hpp"
#include "components/GameGraphElements.hpp"
#include "imgui.h"
#include "imgui_stdlib.h"
#include <format>

namespace components {
Repeater::Repeater(
    std::string name,
    std::optional<unsigned int> max_in,
    std::optional<unsigned int> max_out)
    : components::OComponent(name)
    , m_d { { } }
{
    m_d->max_in = max_in;
    m_d->max_out = max_out;
}
Repeater::Repeater(
    fl::string_param_t name)
    : components::OComponent(std::string(name.data()))
    , m_d { { } }
{
}
Repeater::Repeater(Repeater&& o)
    : components::OComponent(std::move(o))
    , m_d { std::move(o.m_d) }
{
}
Repeater& Repeater::operator=(Repeater&& o)
{
    if (&o == this)
        return *this;
    components::OComponent::operator=(std::move(o));
    m_d = std::move(o.m_d);
    return *this;
}
Repeater::~Repeater() { }
void Repeater::draw()
{
    // const auto center = get_center();
    ::DrawRectangleRounded(
        this->m_bounds,
        0.2,
        10,
        BODY_COLOR);
}
void Repeater::update()
{
}
machine::actor::Actor<void> Repeater::poll(machine::Mctx ctx)
{
    while (1) {
        const auto [s, m] = co_await ctx.recv();
        if (!m_d->from.contains(s))
            continue;
        for (const auto& to : m_d->to) {
            auto cpy = std::any(m);
            co_await ctx.send(to, std::move(cpy));
        }
    }
}

void Repeater::set_size(const ::Vector2& sz)
{
    this->m_bounds.width = sz.x;
    this->m_bounds.height = sz.y;
}
std::any Repeater::on_incoming_connection(
    std::string_view n,
    const machine::Connection*,
    std::any)
{
    if (m_d->max_in && m_d->max_in < m_d->from.size() + 1) {
        throw std::runtime_error(
            std::format("repeater '{}': maximum in capacity exceeded"
                        " with connection '{}'!",
                get_name(),
                n));
    }
    m_d->from.insert(std::string(n));
    return nullptr;
}

std::any Repeater::on_outcoming_connection(
    std::string_view n,
    const machine::Connection*,
    std::any)
{
    if (m_d->max_out && m_d->max_out < m_d->to.size() + 1) {
        throw std::runtime_error(
            std::format("repeater '{}': maximum out capacity exceeded"
                        " with connection '{}'!",
                get_name(),
                n));
    }
    m_d->to.insert(std::string(n));
    return nullptr;
}
const char*
Repeater::marshall_to_xml_name() const noexcept
{
    return "repeater";
}
void Repeater::marshall_to_xml(pugi::xml_node& self) const noexcept
{
    OComponent::marshall_to_xml(self);
    if (m_d->max_in) {
        self.append_attribute("max_in").set_value(*m_d->max_in);
    }
    if (m_d->max_out) {
        self.append_attribute("max_out").set_value(*m_d->max_out);
    }
    append_size_attributes(self, get_size());
}
std::unique_ptr<game::Editor> Repeater::get_editor_object()
{
    auto comp_editor = OComponent::get_editor_object();
    int _max_in = m_d->max_in.value_or(m_d->from.size());
    int _max_out = m_d->max_out.value_or(m_d->to.size());
    std::string _from { }, _to { };
    return game::LambdaEditor::make([=,
                                        this,
                                        comp_editor = std::move(comp_editor)] mutable {
        bool flag = false;
        auto& from = m_d->from;
        auto& to = m_d->to;
        if (ImGui::CollapsingHeader("repeater")) {
            ImGui::SeparatorText("general");
            if (ImGui::InputInt("Max in", &_max_in)) {
                // evil fucking multiple assignment
                m_d->max_in
                    = _max_in
                    = std::max(_max_in, static_cast<int>(from.size()));
                flag = true;
            }
            if (ImGui::InputInt("Max out", &_max_out)) {
                m_d->max_out
                    = _max_out
                    = std::max(_max_out, static_cast<int>(to.size()));
                flag = true;
            }
            ImGui::SeparatorText("connections");
            const auto conns = [&](
                                   std::string_view tnode,
                                   std::set<std::string>& set,
                                   std::optional<unsigned int>& sz,
                                   std::string& buf) {
                std::optional<const std::string&> rm;
                ImGui::PushID(&set);
                if (ImGui::TreeNode(tnode.data())) {
                    if (ImGui::BeginTable("Connections", 2,
                            ImGuiTableFlags_Resizable
                                | ImGuiTableFlags_NoSavedSettings
                                | ImGuiTableFlags_Borders)) {
                        for (const auto& c : set) {
                            ImGui::PushID(&c);
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text("%s", c.c_str());
                            ImGui::TableNextColumn();
                            if (ImGui::Button("Remove")) {
                                rm = c;
                                flag = true;
                            }
                            ImGui::PopID();
                        }
                        ImGui::EndTable();
                    }
                    ImGui::TreePop();
                    if ((!sz || (sz && set.size() < *sz))) {
                        ImGui::InputText("Connection", &buf);
                        if (!buf.empty() && ImGui::Button("Add")) {
                            flag = true;
                            set.insert(buf);
                        }
                    }
                }
                ImGui::PopID();
                if (rm) {
                    set.erase(*rm);
                }
            };
            conns("recieve from", from, m_d->max_in, _from);
            conns("send to", to, m_d->max_out, _to);
        }
        flag |= comp_editor->draw_update();
        return flag;
    });
}
}
