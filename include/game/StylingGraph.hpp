#pragma once
#include "components/GameGraphElements.hpp"
#include "game/StylingContext.hpp"
#include "machine/MachineGraph.hpp"
#include <game/Styling.hpp>
#include <memory>
#include <queue>
#include <raylib.h>
#include <set>
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

public:
    struct Deps {
    private:
        std::vector<elem_id_t> ids { };
        std::set<elem_id_t> dep_hash { };

    public:
        auto check(const elem_id_t&) const -> bool;
        auto insert(const elem_id_t&) -> bool;
        auto remove(const elem_id_t&) -> void;
        auto clear() -> void;
        auto get_dep_list() -> const std::vector<elem_id_t>&;
        Deps() = default;
        Deps(const Deps&) = delete;
        Deps& operator=(const Deps&) = delete;
        Deps(Deps&&) = default;
        Deps& operator=(Deps&&) = default;
    };

private:
    std::unordered_map<elem_id_t, Style::shared_ptr_t> m_styles { };
    std::unordered_map<elem_id_t, Deps> m_dependent { };
    Style::shared_ptr_t m_default_style;
    std::unordered_map<elem_id_t, std::queue<elem_id_t>> m_notify_queue_cache { };
    std::unique_ptr<StyleSchemeWrap> m_style_scm { };
    std::optional<elem_id_t> m_currently_styling { };

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
    /// styling with dependency tracing on query calls
    auto style_element_with_dep(const elem_id_t&) -> void;
    auto try_invalidate_cache_for(const elem_id_t&) -> void;
    auto try_invalidate_cache_for_current() -> void;

public:
    auto register_style(const std::string&, std::shared_ptr<Style>) -> void;
    auto style_notify_for(const std::string&) -> void;
    auto style_notify_all() -> void;
    auto try_add_dep_to_current(std::optional<const std::string&>) -> void;
    virtual auto query_style(const std::string&, bool no_dep = false)
        -> std::optional<const game::Style&> override;
    virtual auto query_component(const std::string&, bool)
        -> std::optional<const components::OComponent&> override;
    virtual auto query_connection(const std::string&, bool)
        -> std::optional<const components::OConnection&> override;
    virtual game::StyleSchemeWrap& get_style_scheme() override;
    virtual auto exists(const std::string&) const -> bool override;
    virtual auto is_connector(const std::string&) const -> bool override;
    virtual auto is_component(const std::string&) const -> bool override;
};
}
