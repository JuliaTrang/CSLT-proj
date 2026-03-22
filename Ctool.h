#pragma once
#include <SFML/Graphics.hpp>
#include "library.h"
class ctool {
private:
    sf::RenderWindow* console_window; // chính là cửa sổ game
    sf::Vector2u window_size;
    sf::Clock clock;

public:
    ctool(sf::RenderWindow* window);

    short getWidthConsoleScreen();
    short getHeightConsoleScreen();

    void sleepMillisecond(int ms);
    void restartClock();
    float getElapsedTimeSeconds();
    sf::Time getElapsedTime();
};