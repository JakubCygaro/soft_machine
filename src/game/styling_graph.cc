#include "game/Styling.hpp"
#include "game/StylingGraph.hpp"
#include <queue>
#include <set>

#define LOG_PREFIX "[STYLING GRAPH] "

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
    m_notify_queue_cache.clear();
    register_style(conn->get_name(), m_default_style);
    style_notify_all();
}
void StylingGraph::on_create_component(elem_id_t, comp_t* comp)
{
    m_notify_queue_cache.clear();
    register_style(comp->get_name(), m_default_style);
    style_notify_all();
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
    ::TraceLog(::LOG_DEBUG,
        LOG_PREFIX "BUILDING NOTIFY QUEUE FOR '%s':",
        *get_name_of(id)->c_str());
    std::set<elem_id_t> cycle_hash { id };
    std::queue<elem_id_t> notify_queue { };
    notify_queue.push(id);

    std::queue<elem_id_t> dependents { };
    for (auto& d : m_dependent[id].get_dep_list()) {
        if (graph_t::exists(d)) {
            dependents.push(d);
        }
    }
    while (!dependents.empty()) {
        auto did = dependents.front();
        dependents.pop();
        if (cycle_hash.contains(did)) {
            ::TraceLog(::LOG_ERROR,
                LOG_PREFIX "Dependency cycle for %s",
                *get_name_of(id)->c_str());
            return std::nullopt;
        } else
            cycle_hash.insert(did);
        for (auto& d2 : m_dependent[did].get_dep_list()) {
            dependents.push(d2);
        }
        notify_queue.push(did);
        ::TraceLog(::LOG_DEBUG, " -> '%s'", get_name_of(did)->c_str());
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
    // auto notify_queue_opt = build_notify_queue_for(id);
    if (!notify_queue_opt.has_value())
        return;
    auto notify_queue = std::queue(*notify_queue_opt);
    using namespace std::ranges;
    while (!notify_queue.empty()) {
        auto nid = notify_queue.front();
        notify_queue.pop();
        style_element_with_dep(nid);
        if (graph_t::is_component(nid))
            notify_incident_of_component(nid);
    }
    std::flush(std::cout);
}
auto StylingGraph::style_notify_all() -> void
{
    m_notify_queue_cache.clear();
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
auto StylingGraph::try_invalidate_cache_for(const elem_id_t& id) -> void
{
    m_notify_queue_cache.erase(id);
}
auto StylingGraph::try_invalidate_cache_for_current() -> void
{
    if (auto nid = m_currently_styling; nid)
        try_invalidate_cache_for(*nid);
}
auto StylingGraph::try_add_dep_to_current(
    std::optional<const std::string&> name) -> void
{
    if (auto nid = m_currently_styling; !nid)
        return;
    auto nid = *m_currently_styling;
    if (name) {
        if (m_dependent.at(name_to_id(*name)).insert(nid))
            try_invalidate_cache_for(name_to_id(*name));
    }
}
auto StylingGraph::query_style(const std::string& name, bool no_dep)
    -> std::optional<const game::Style&>
{
    ::TraceLog(::LOG_DEBUG,
        LOG_PREFIX "QUERY FROM '%s', FOR STYLE OF '%s', NO_DEP = %s",
        get_name_of(*m_currently_styling)->c_str(),
        name.c_str(),
        no_dep ? "YES" : "NO");
    if (!exists(name))
        return { };
    const auto id = name_to_id(name);
    std::shared_ptr<Style> st = nullptr;
    if (!m_styles.contains(id)) {
        ::TraceLog(::LOG_ERROR,
            LOG_PREFIX "ELEMENT '%s' DOES NOT HAVE A STYLE REGISTERED",
            name.c_str());
    } else {
        st = m_styles.at(id);
    }
    if (!no_dep)
        try_add_dep_to_current(st ? std::optional(std::ref(name)) : std::nullopt);
    if (st)
        return *st;
    return { };
}
auto StylingGraph::query_component(const std::string& name, bool no_dep)
    -> std::optional<const components::OComponent&>
{
    ::TraceLog(::LOG_DEBUG,
        LOG_PREFIX "QUERY FROM '%s', FOR COMPONENT '%s', NO_DEP = %s",
        get_name_of(*m_currently_styling)->c_str(),
        name.c_str(),
        no_dep ? "YES" : "NO");
    auto comp = graph_t::query_component(name);
    if (!no_dep)
        try_add_dep_to_current(comp ? std::optional(std::ref(name)) : std::nullopt);
    if (comp) {
        return *comp.value();
    }
    return { };
}
auto StylingGraph::query_connection(const std::string& name, bool no_dep)
    -> std::optional<const components::OConnection&>
{
    ::TraceLog(::LOG_DEBUG,
        LOG_PREFIX "QUERY FROM '%s', FOR CONNECTION '%s', NO_DEP = %s",
        get_name_of(*m_currently_styling)->c_str(),
        name.c_str(),
        no_dep ? "YES" : "NO");
    auto conn = graph_t::query_connection(name);
    if (!no_dep)
        try_add_dep_to_current(conn ? std::optional(std::ref(name)) : std::nullopt);
    if (conn) {
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
