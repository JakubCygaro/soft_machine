#include "game/Styling.hpp"
#include "game/StylingGraph.hpp"
#include <algorithm>
#include <queue>
#include <set>

namespace game {

StylingGraph::StylingGraph(Style::shared_ptr_t default_style)
    : game::StylingGraph::graph_t()
    , m_default_style { default_style }
{
}
StylingGraph::StylingGraph(StylingGraph&& o)
    : StylingGraph::graph_t(std::move(o))
    , m_styles { std::move(o.m_styles) }
    , m_dependent { std::move(o.m_dependent) }
    , m_default_style { std::move(o.m_default_style) }
{
}
StylingGraph& StylingGraph::operator=(StylingGraph&& o)
{
    StylingGraph::graph_t::operator=(std::move(o));
    m_styles = std::move(o.m_styles);
    m_dependent = std::move(o.m_dependent);
    m_default_style = std::move(o.m_default_style);
    return *this;
}
StylingGraph* StylingGraph::create(Style::shared_ptr_t default_style)
{
    return new StylingGraph(default_style);
}
void StylingGraph::on_create_connection(elem_id_t, conn_t* conn)
{
    conn->set_styling(m_default_style);
}
void StylingGraph::on_create_component(elem_id_t, comp_t* comp)
{
    comp->set_styling(m_default_style);
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
    for (const auto& dep : s->get_dependencies()) {
        if (!exists(dep))
            continue;
        auto dep_id = name_to_id(dep);
        m_dependent[id].push_back(dep_id);
    }
    style_notify_for(id);
}
auto StylingGraph::style_notify_for(
    const elem_id_t& id) -> void
{
    if (!m_styles.contains(id))
        return;
    std::set<elem_id_t> cycle_hash { id };
    std::queue<elem_id_t> notify_queue { };
    notify_queue.push(id);

    std::queue<elem_id_t> dependents { };
    for (auto& d : m_dependent[id]) {
        if (exists(d))
            dependents.push(d);
    }
    while (!dependents.empty()) {
        auto did = dependents.front();
        dependents.pop();
        if (cycle_hash.contains(did))
            return;
        else
            cycle_hash.insert(did);
        for (auto& d2 : m_dependent[did]) {
            dependents.push(d2);
        }
        notify_queue.push(did);
    }
    while (!notify_queue.empty()) {
        auto nid = notify_queue.front();
        notify_queue.pop();
        m_styles[nid]->style(get_element_as<Stylable>(nid));
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
}
