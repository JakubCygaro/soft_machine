#include "facelift/State.hpp"
#include <cstring>
#include <ranges>
#include <raylib.h>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
    auto args = std::ranges::views::iota(0, argc)
        | std::ranges::views::transform([=](auto idx) {
              return std::string(argv[idx], std::strlen(argv[idx]));
          })
        | std::ranges::views::drop(1)
        | std::ranges::to<std::vector>();
    facelift::init(args);
    while (!::WindowShouldClose()) {
        facelift::update();
        facelift::draw();
    }
    facelift::deinit();
}
