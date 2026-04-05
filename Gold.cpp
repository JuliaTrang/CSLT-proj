#include "Gold.h"

Gold::Gold() : gold(1000) { // khởi đầu với 1000 vàng
    font.loadFromFile("Fonts/pixel.ttf");
    coinTexture.loadFromFile("TileSets/tower/money.png");

    coin.setTexture(coinTexture);
    coin.setScale(0.1f, 0.1f);

    goldText.setFont(font);
    goldText.setCharacterSize(36);
    goldText.setFillColor(sf::Color::White);
    goldText.setString(std::to_string(gold));
}

void Gold::setGold(int amount) {
    gold = amount;
    goldText.setString(std::to_string(gold));
}

int Gold::getGold() const {
    return gold;
}

void Gold::addGold(int amount) {
    gold += amount;
    goldText.setString(std::to_string(gold));
}

bool Gold::spendGold(int amount) {
    if (gold >= amount) {
        gold -= amount;
        goldText.setString(std::to_string(gold));
        return true;
    }
    return false;
}

void Gold::setPosition(float x, float y) {
    goldText.setPosition(x + 45.f, y);
    coin.setPosition(x, y);
}

void Gold::draw(sf::RenderWindow& window, bool backgroundBox) {
    if (backgroundBox) {
        sf::RectangleShape box(sf::Vector2f(160, 60));
        box.setPosition(coin.getPosition().x - 15.f, coin.getPosition().y - 5.f);
        box.setFillColor(sf::Color(0, 0, 0, 180));
        box.setOutlineThickness(2.f);
        box.setOutlineColor(sf::Color::White);
        window.draw(box);
    }
    window.draw(coin);
    window.draw(goldText);
}
