#pragma once
#include <SFML/Graphics.hpp>

class Gold {
private:
    int gold;
    sf::Font font;
    sf::Texture coinTexture;
    sf::Sprite coin;
    sf::Text goldText;
public:
    Gold();
    void setGold(int amount);
    int getGold() const;
    void addGold(int amount);
    bool spendGold(int amount); // Trả về false nếu không đủ vàng
    void setPosition(float x, float y);
    void draw(sf::RenderWindow& window, bool backgroundBox = false);
};
