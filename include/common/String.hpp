#pragma once
#include <string>
#include <string_view>

namespace common {
std::string ltrim(const std::string&);
std::string ltrim(const std::string_view);
std::string rtrim(const std::string&);
std::string rtrim(const std::string_view);
std::string trim(const std::string&);
std::string trim(const std::string_view);
namespace str {
    bool equal(const std::string_view, const std::string_view);
    bool equal(const std::string&, const std::string&);
}
}
