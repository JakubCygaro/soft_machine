#pragma once
#include "common/Result.hpp"
#include "components/GameGraphElements.hpp"
#include "machine/MachineGraph.hpp"
#include <pugixml.hpp>
#ifndef CLANGD_SKIP
#include <meta>
#endif

namespace game {
Result<std::runtime_error, Unit>
populate_machine_from_xml(
    machine::MachineGraph<components::OComponent, components::OConnection>&,
    const std::string&);
Result<std::runtime_error, std::string>
attribute_as_string(
    const pugi::xml_node&,
    const std::string&);

}
