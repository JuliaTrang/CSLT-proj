#pragma once
#include <SFML/Graphics.hpp>
#include "Bullet.h"
#include "Enemy.h"
#include "Cpoint.h"
#include <vector>

class _Tower {
private:
    const sf::Texture* texture; // Dùng tham chiếu
    sf::Sprite sprite;
    sf::CircleShape attackRadiusShape;
    sf::Clock attackClock;
    float attackCooldown;
    float attackRadius;
    int gridX, gridY;
    vector<Bullet> bullets;
    int towersIndex = 0;
public:
    _Tower(int gridX, int gridY, float radius, float cooldown, const sf::Texture* texture);
    _Tower(int gridX, int gridY, float radius, float cooldown, const sf::Texture* texture, bool fromSave, int towerIdx);
    _Tower(const _Tower& other); // <-- Thêm
    _Tower& operator=(const _Tower& other); // <-- Thêm
    void setBulletTexture(const sf::Texture& texture);
    sf::Texture bulletTexture; // hoặc const sf::Texture* bulletTexture;

    void update(std::vector<Enemy>& enemies, float deltaTime);
    void draw(sf::RenderWindow& window);
    void setPosition(int x, int y);
    Cpoint getPosition() const;
    float getAttackRadius() const;
    bool isNewlyPlaced = false;

    void setTowersIndex(int i);
    int getIndex() const;
    sf::Sprite& getSprite() { return sprite; }
};