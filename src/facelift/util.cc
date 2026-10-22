#include "facelift/State.hpp"
#include "game/Xml.hpp"
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
namespace facelift {
namespace fs = std::filesystem;
std::expected<void, std::string> load_file(std::string& p)
{
    if (!fs::exists(p) || !fs::is_regular_file(p))
        throw std::unexpected(
            std::format("file '{}' does not exist or is not a regular file", p));
    fl_state.file_name = p;
    std::string xml;
    std::ifstream ifs(fl_state.file_name);
    std::stringstream buf;
    buf << ifs.rdbuf();
    xml = buf.str();
    auto modify = [&](game::GraphScene::graph_t* g) -> auto {
        g->clear();
        auto res = game::populate_machine_from_xml(*g, xml);
        return res;
    };
    auto res = fl_state.graph_scene
                   ->modify_graph<Result<std::runtime_error, Unit>>(modify);
    if (res.iserr()) {
        fl_state.graph_scene->get_graph()->clear();
        return std::unexpected(res.unwrap_err().what());
    }
    return { };
}
}
