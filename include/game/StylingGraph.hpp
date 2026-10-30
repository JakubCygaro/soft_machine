#pragma once
#include "components/GameGraphElements.hpp"
#include "game/StylingContext.hpp"
#include "machine/MachineGraph.hpp"
#include <game/Styling.hpp>
#include <memory>
#include <queue>
#include <raylib.h>
#include <string>
#include <unordered_map>
namespace game {
class StylingGraph : public machine::MachineGraph<
                         components::OComponent,
                         components::OConnection>,
                     public game::StylingContext {
public:
private:
    using graph_t = machine::MachineGraph<
        components::OComponent,
        components::OConnection>;
    std::unordered_map<elem_id_t, Style::shared_ptr_t> m_styles { };
    std::unordered_map<elem_id_t, std::vector<elem_id_t>> m_dependent { };
    Style::shared_ptr_t m_default_style;
    std::unordered_map<elem_id_t, std::queue<elem_id_t>> m_notify_queue_cache { };

protected:
    StylingGraph(Style::shared_ptr_t default_style);

public:
    StylingGraph(const StylingGraph&) = delete;
    StylingGraph& operator=(const StylingGraph&) = delete;
    StylingGraph(StylingGraph&&);
    StylingGraph& operator=(StylingGraph&&);
    static StylingGraph* create(Style::shared_ptr_t default_style);

protected:
    virtual void on_create_connection(elem_id_t, conn_t*) override;
    virtual void on_create_component(elem_id_t, comp_t*) override;
    virtual void on_removing_element(
        const std::string& name,
        const elem_id_t& id) override;

private:
    auto style_notify_for(const elem_id_t&) -> void;
    auto build_notify_queue_for(const elem_id_t&) -> std::optional<
        const std::queue<elem_id_t>&>;
    auto notify_incident_of_component(const elem_id_t&) -> void;

public:
    auto register_style(const std::string&, std::shared_ptr<Style>) -> void;
    auto style_notify_for(const std::string&) -> void;
    auto style_notify_all() -> void;
    virtual auto query_component(const std::string&) const
        -> std::optional<const components::OComponent&> override;
    virtual auto query_connection(const std::string&) const
        -> std::optional<const components::OConnection&> override;
};
}
