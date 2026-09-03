#include "facelift/State.hpp"
#include <raylib.h>

int main(void)
{
    facelift::init();
    while (!::WindowShouldClose()) {
        facelift::update();
        facelift::draw();
    }
    facelift::deinit();
}
