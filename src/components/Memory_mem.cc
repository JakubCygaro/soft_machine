#include "components/Memory.hpp"
namespace components::mem {
MemoryWrite::MemoryWrite(size_t at, int val)
    : at { at }
    , val { val }
{
}
std::string MemoryWrite::type_name() const
{
    return "Memory write request message";
}
std::string MemoryWrite::to_display_string() const
{
    return std::format("MemWrite {{ at: {} val {} }}", at, val);
}
MemoryRead::MemoryRead(size_t at)
    : at { at }
{
}
std::string MemoryRead::type_name() const
{
    return "Memory read request message";
}
std::string MemoryRead::to_display_string() const
{
    return std::format("MemRead {{ at: {} }}", at);
}
MemoryResponse::MemoryResponse(int val)
    : val { val }
{
}
std::string MemoryResponse::type_name() const
{
    return "Memory read response message";
}
std::string MemoryResponse::to_display_string() const
{
    return std::format("MemRead {{ val: {} }}", val);
}
MemoryFail::MemoryFail(size_t at)
    : at { at }
{
}
std::string MemoryFail::type_name() const
{
    return "Memory read failure message";
}
std::string MemoryFail::to_display_string() const
{
    return std::format("MemReadFail {{ at: {} }}", at);
}
}
