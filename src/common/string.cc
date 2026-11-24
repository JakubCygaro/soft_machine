#include "common/String.hpp"
#include <algorithm>

namespace common {
std::string ltrim(const std::string_view sv)
{
    return std::string(
        std::find_if(
            sv.begin(),
            sv.end(),
            [](unsigned char ch) {
                return !std::isspace(ch);
            }),
        sv.end());
}
std::string ltrim(const std::string& s)
{
    return ltrim(std::string_view { s });
}
std::string rtrim(const std::string_view sv)
{
    return std::string(
        sv.begin(),
        std::find_if(
            sv.rbegin(),
            sv.rend(),
            [](unsigned char ch) {
                return !std::isspace(ch);
            })
            .base());
}
std::string rtrim(const std::string& s)
{
    return rtrim(std::string_view { s });
}
std::string trim(const std::string_view sv)
{
    return ltrim(rtrim(sv));
}
std::string trim(const std::string& s)
{
    return ltrim(rtrim(std::string_view { s }));
}
namespace str {
    bool equal(const std::string_view sva, const std::string_view svb)
    {
        return sva.compare(svb);
    }
    bool equal(const std::string& sa, const std::string& sb)
    {
        return equal(std::string_view { sa }, sb);
    }
}
}
