#pragma once
#include <SFML/Graphics.hpp>

class VolumeSlider {
private:
    sf::RectangleShape track;
    sf::RectangleShape handle;
    float* volumeRef;

public:
    VolumeSlider(float* volumePtr);
    void setPosition(float x, float y);
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
    void updateVolumeFromMouse(sf::Vector2f pos);
    bool isDragging = false;
};


