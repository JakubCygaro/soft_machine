#pragma once

#include <algorithm>
#include <string>
namespace common {
inline std::string ltrim(const std::string& s)
{
    return std::string(
        std::find_if(
            s.begin(),
            s.end(),
            [](unsigned char ch) {
                return !std::isspace(ch);
            }),
        s.end());
}

inline std::string rtrim(const std::string& s)
{
    return std::string(
        s.begin(),
        std::find_if(
            s.rbegin(),
            s.rend(),
            [](unsigned char ch) {
                return !std::isspace(ch);
            })
            .base());
}
inline std::string trim(const std::string& s)
{
    return ltrim(rtrim(s));
}
}
