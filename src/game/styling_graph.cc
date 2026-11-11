#include "game/Styling.hpp"
#include "game/StylingGraph.hpp"
#include <queue>
#include <set>

namespace game {

StylingGraph::StylingGraph(Style::shared_ptr_t default_style)
    : graph_t()
    , m_default_style { default_style }
    , m_style_scm { std::make_unique<StyleSchemeWrap>() }
{
}
StylingGraph::StylingGraph(StylingGraph&& o)
    : graph_t(std::move(o))
    , m_styles { std::move(o.m_styles) }
    , m_dependent { std::move(o.m_dependent) }
    , m_default_style { std::move(o.m_default_style) }
    , m_notify_queue_cache { std::move(o.m_notify_queue_cache) }
    , m_style_scm { std::move(o.m_style_scm) }
{
}
StylingGraph& StylingGraph::operator=(StylingGraph&& o)
{
    graph_t::operator=(std::move(o));
    m_styles = std::move(o.m_styles);
    m_dependent = std::move(o.m_dependent);
    m_default_style = std::move(o.m_default_style);
    m_notify_queue_cache = std::move(o.m_notify_queue_cache);
    m_style_scm = std::move(o.m_style_scm);
    return *this;
}
StylingGraph* StylingGraph::create(Style::shared_ptr_t default_style)
{
    return new StylingGraph(default_style);
}
void StylingGraph::on_create_connection(elem_id_t, conn_t* conn)
{
    conn->set_styling(m_default_style);
    m_default_style->style(this, conn);
}
void StylingGraph::on_create_component(elem_id_t, comp_t* comp)
{
    comp->set_styling(m_default_style);
    m_default_style->style(this, comp);
}
auto StylingGraph::notify_incident_of_component(const elem_id_t& id) -> void
{
    const auto* elem_p = get_component(id);
    if (auto inc = get_incident_to(id); inc) {
        for (auto* conn : **inc) {
            conn->on_component_changed(
                elem_p == conn->get_end()
                    ? components::OConnection::Attachment::End
                    : components::OConnection::Attachment::Start);
        }
    }
}
void StylingGraph::on_removing_element(
    const std::string& name,
    const elem_id_t& id)
{
    (void)name;
    m_styles.erase(id);
}

auto StylingGraph::register_style(
    const std::string& name,
    std::shared_ptr<Style> s) -> void
{
    if (!exists(name))
        return;
    auto id = name_to_id(name);
    Object* element = get_element_as<Object>(id);
    m_styles[id] = s;
    element->set_styling(s);
    m_dependent[id] = { };
    style_notify_for(id);
}

auto StylingGraph::build_notify_queue_for(const elem_id_t& id)
    -> std::optional<const std::queue<elem_id_t>&>
{
    std::set<elem_id_t> cycle_hash { id };
    std::queue<elem_id_t> notify_queue { };
    notify_queue.push(id);

    std::queue<elem_id_t> dependents { };
    for (auto& d : m_dependent[id].get_dep_list()) {
        if (graph_t::exists(d))
            dependents.push(d);
    }
    while (!dependents.empty()) {
        auto did = dependents.front();
        dependents.pop();
        if (cycle_hash.contains(did))
            return std::nullopt;
        else
            cycle_hash.insert(did);
        for (auto& d2 : m_dependent[did].get_dep_list()) {
            dependents.push(d2);
        }
        notify_queue.push(did);
    }
    if (m_notify_queue_cache.size() >= 16) {
        auto first = m_notify_queue_cache.begin()->first;
        m_notify_queue_cache.erase(first);
    }
    m_notify_queue_cache[id] = std::move(notify_queue);
    return m_notify_queue_cache.at(id);
}
auto StylingGraph::style_element_with_dep(const elem_id_t& nid) -> void
{
    m_currently_styling = nid;
    if (m_styles[nid]->should_rebuild_dep_graph()) {
        m_dependent[nid].clear();
    }
    m_styles[nid]->style(this, get_element_as<Stylable>(nid));
    m_currently_styling = std::nullopt;
}
auto StylingGraph::style_notify_for(
    const elem_id_t& id) -> void
{
    if (!m_styles.contains(id))
        return;
    auto notify_queue_opt = m_notify_queue_cache.contains(id)
        ? std::optional<const std::queue<elem_id_t>&>(m_notify_queue_cache.at(id))
        : build_notify_queue_for(id);
    if (!notify_queue_opt.has_value())
        return;
    auto notify_queue = std::queue(*notify_queue_opt);

    while (!notify_queue.empty()) {
        auto nid = notify_queue.front();
        notify_queue.pop();
        style_element_with_dep(nid);
        if (graph_t::is_component(nid))
            notify_incident_of_component(nid);
    }
}
auto StylingGraph::style_notify_all() -> void
{
    for (const auto& [elem_id, st] : m_styles) {
        style_notify_for(elem_id);
    }
}
auto StylingGraph::style_notify_for(
    const std::string& name) -> void
{
    if (!exists(name))
        return;
    const auto id = name_to_id(name);
    style_notify_for(id);
}

auto StylingGraph::try_add_dep_to_current(const std::string& name) -> void
{
    if (auto nid = m_currently_styling; nid) {
        if (m_dependent.at(name_to_id(name)).insert(*nid))
            m_notify_queue_cache.erase(name_to_id(name));
    }
}
auto StylingGraph::query_component(const std::string& name, bool no_dep)
    -> std::optional<const components::OComponent&>
{
    if (auto comp = graph_t::query_component(name); comp) {
        if (!no_dep)
            try_add_dep_to_current(name);
        return *comp.value();
    }
    return { };
}
auto StylingGraph::query_connection(const std::string& name, bool no_dep)
    -> std::optional<const components::OConnection&>
{
    if (auto conn = graph_t::query_connection(name); conn) {
        if (!no_dep)
            try_add_dep_to_current(name);
        return *conn.value();
    }
    return { };
}
game::StyleSchemeWrap& StylingGraph::get_style_scheme()
{
    return *m_style_scm;
}
auto StylingGraph::exists(const std::string& n) const -> bool
{
    return graph_t::exists(n);
}
auto StylingGraph::is_connector(const std::string& n) const -> bool
{
    return graph_t::is_connector(n);
}
auto StylingGraph::is_component(const std::string& n) const -> bool
{
    return graph_t::is_component(n);
}
auto StylingGraph::Deps::check(const elem_id_t& id) const -> bool
{
    return dep_hash.contains(id);
}
auto StylingGraph::Deps::insert(const elem_id_t& id) -> bool
{
    if (!check(id)) {
        ids.push_back(id);
        return std::get<1>(dep_hash.insert(id));
    }
    return false;
}
auto StylingGraph::Deps::remove(const elem_id_t& id) -> void
{
    if (check(id)) {
        ids.erase(std::find(ids.begin(), ids.end(), id));
        dep_hash.erase(id);
    }
}
auto StylingGraph::Deps::clear() -> void
{
    ids.clear();
    dep_hash.clear();
}

auto StylingGraph::Deps::get_dep_list() -> const std::vector<elem_id_t>&
{
    return ids;
}
}
