#pragma once
#include <SFML/Graphics.hpp>
#include "Enemy.h"

class Bullet {
private:
    int damage;
    float speed;
    sf::Sprite sprite;
    sf::Texture texture;
    float aliveTime;
    sf::Vector2f position;
    sf::Vector2f direction;
    Enemy* target;
    bool active;
    sf::Vector2f velocity;
public:
    Bullet();
    Bullet(int dmg, float spd, const sf::Texture& tex);

    void shoot(const sf::Vector2f& startPos, Enemy* tgt);
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);

    bool isActive() const;
    bool hasHitTarget() const;
    Enemy* getTarget() const;
};

