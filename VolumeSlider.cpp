#include "VolumeSlider.h"
#include <algorithm>

VolumeSlider::VolumeSlider(float* volumePtr) : volumeRef(volumePtr) {
    track.setSize({ 350.f, 14.f });  // ngang
    track.setFillColor(sf::Color(150, 150, 150));

    handle.setSize({ 14.f, 28.f });  // dọc
    handle.setFillColor(sf::Color::White);
    setPosition(0, 0);
}

void VolumeSlider::setPosition(float x, float y) {
    track.setPosition(x, y);
    float percent = *volumeRef / 100.f;
    handle.setPosition(x + percent * track.getSize().x, y - 5.f);
}

void VolumeSlider::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    auto mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        if (handle.getGlobalBounds().contains(worldPos) || track.getGlobalBounds().contains(worldPos)) {
            isDragging = true;
            updateVolumeFromMouse(worldPos);
        }
    }
    else if (event.type == sf::Event::MouseMoved && isDragging) {
        updateVolumeFromMouse(worldPos);
    }
    else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        isDragging = false;
    }
}

void VolumeSlider::draw(sf::RenderWindow& window) {
    window.draw(track);
    window.draw(handle);
}

void VolumeSlider::updateVolumeFromMouse(sf::Vector2f pos)
{
    auto bounds = track.getGlobalBounds();
    float percent = (pos.x - bounds.left) / bounds.width;
    percent = std::max(0.f, std::min(1.f, percent));
    *volumeRef = percent * 100.f;
    handle.setPosition(bounds.left + percent * bounds.width - handle.getSize().x / 2.f, bounds.top - 6.f);
}
