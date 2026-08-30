#include "game/Scene.hpp"
#include "machine/MachineGraph.hpp"
#include "rlImGui.h"
#include <memory>
#include <raylib.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

int main(void)
{
    ::SetConfigFlags(
        ::FLAG_WINDOW_RESIZABLE);
    ::InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Facelift");
    ::rlImGuiSetup(true);
    auto mg = std::unique_ptr<machine::MachineGraph>(
        machine::MachineGraph::create());
    auto sc = game::GraphScene(std::move(mg),
        { 0,
            0,
            SCREEN_WIDTH,
            SCREEN_HEIGHT });
    while (!::WindowShouldClose()) {
        if (::IsWindowResized()) {
            auto w = ::GetScreenWidth();
            auto h = ::GetScreenHeight();
            ::SetWindowSize(w, h);
            sc.bounds.width = w;
            sc.bounds.height = h;
        }
        sc.update();
        ::BeginDrawing();
        ::rlImGuiBegin();
        sc.draw();
        ::rlImGuiEnd();
        ::EndDrawing();
    }
    ::rlImGuiShutdown();
    ::CloseWindow();
}

// TODO: component creation dialog, via reflecting over components namespace
// and gathering all constructor parameters
