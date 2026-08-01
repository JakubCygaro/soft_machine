#include "game/Xml.hpp"
#include "components/Passthrough.hpp"

namespace {
using err_t = std::runtime_error;
using ret_t = unit_result_t<err_t>;

using name_from_to_t = std::tuple<std::string, std::string, std::string>;
result_t<err_t, name_from_to_t>
get_name_from_and_to(pugi::xml_node& n)
{
    auto name_a = n.attribute("name");
    if (!name_a)
        return err_t("No 'name' defined for connector");
    auto name = name_a.as_string();
    if (!name)
        return err_t("Empty 'name' field");
    auto from_a = n.attribute("from");
    if (!from_a)
        return err_t("No 'from' defined for connector");
    auto from = from_a.as_string();
    if (!from)
        return err_t("Empty 'from' field");
    auto to_a = n.attribute("to");
    if (!to_a)
        return err_t("No 'to' defined for connector");
    auto to = to_a.as_string();
    if (!to)
        return err_t("Empty 'to' field");
    return name_from_to_t(name, from, to);
}

ret_t build_passthrough(machine::MachineGraph& mg, pugi::xml_node& n)
{
    auto a = get_name_from_and_to(n);
    if (result::iserr(a))
        return std::get<err_t>(a);
    auto [name, f, t] = std::get<name_from_to_t>(a);
    mg.create_connection<components::Passthrough>(
        name,
        f,
        t);
    return unit();
}
ret_t build_from_xml_node(machine::MachineGraph& mg, pugi::xml_node& n)
{
    auto name = n.name();
    if (std::string("passthrough").compare(name)) {
        auto res = build_passthrough(mg, n);
        if (result::iserr(res))
            return std::get<err_t>(res);
    }
    return unit();
}
}
namespace game {
result_t<std::runtime_error, Unit>
populate_machine_from_xml(machine::MachineGraph& mg, const std::string& xml)
{
    pugi::xml_document doc;
    pugi::xml_parse_result res = doc.load_string(xml.c_str());
    if (!res)
        return std::runtime_error(res.description());
    auto graph_node = doc.child("graph");
    if (!graph_node)
        return std::runtime_error(
            "graph node is not present in the xml schema");
    for (auto elem : graph_node.children()) {
        auto res = build_from_xml_node(mg, elem);
        if (result::iserr(res)) {
            return std::get<err_t>(res);
        }
    }
    return unit();
}
}
