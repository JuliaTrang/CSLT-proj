#include "Ctool.h"


ctool::ctool(sf::RenderWindow* window) {
    console_window = window;
    window_size = console_window->getSize();
}

short ctool::getWidthConsoleScreen() {
    return static_cast<short>(window_size.x);
}

short ctool::getHeightConsoleScreen() {
    return static_cast<short>(window_size.y);
}

void ctool::sleepMillisecond(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void ctool::restartClock() {
    clock.restart();
}

float ctool::getElapsedTimeSeconds() {
    return clock.getElapsedTime().asSeconds();
}

sf::Time ctool::getElapsedTime() {
    return clock.getElapsedTime();
}