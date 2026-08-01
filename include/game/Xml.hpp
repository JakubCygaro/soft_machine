#pragma once
#include "common/Result.hpp"
#include "machine/MachineGraph.hpp"
#include <pugixml.hpp>

namespace game {
result_t<std::runtime_error, Unit>
populate_machine_from_xml(machine::MachineGraph&, const std::string&);
}
