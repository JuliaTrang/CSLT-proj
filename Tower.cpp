#include "Tower.h"
#include "map.h"
#include <cmath>
_Tower::_Tower(int gridX, int gridY, float radius, float cooldown, const sf::Texture* texture)
    : gridX(gridX), gridY(gridY), attackRadius(radius), attackCooldown(cooldown), texture(texture)
{
    sprite.setTexture(*texture);
    float scale = float(TILE_SIZE) / texture->getSize().x;
    sprite.setScale(scale, scale);
    sprite.setPosition(gridX * TILE_SIZE, gridY * TILE_SIZE);

    attackRadiusShape.setRadius(attackRadius);
    attackRadiusShape.setOrigin(attackRadius, attackRadius);
    attackRadiusShape.setFillColor(sf::Color(255, 0, 0, 50));
    attackRadiusShape.setPosition(sprite.getPosition());
}

_Tower::_Tower(const _Tower& other)
    : gridX(other.gridX), gridY(other.gridY),
    attackRadius(other.attackRadius),
    attackCooldown(other.attackCooldown),
    texture(other.texture),
    bulletTexture(other.bulletTexture),
    isNewlyPlaced(other.isNewlyPlaced),
    towersIndex(other.towersIndex)
{
    sprite.setTexture(*texture);
    sprite.setPosition(gridX * TILE_SIZE, gridY * TILE_SIZE);
    sprite.setScale(float(TILE_SIZE) / texture->getSize().x, float(TILE_SIZE) / texture->getSize().x);

    attackRadiusShape.setRadius(attackRadius);
    attackRadiusShape.setOrigin(attackRadius, attackRadius);
    attackRadiusShape.setFillColor(sf::Color(255, 0, 0, 50));
    attackRadiusShape.setPosition(sprite.getPosition());
}

_Tower& _Tower::operator=(const _Tower& other)
{
    if (this != &other) {
        gridX = other.gridX;
        gridY = other.gridY;
        attackRadius = other.attackRadius;
        attackCooldown = other.attackCooldown;
        texture = other.texture;
        bulletTexture = other.bulletTexture;
        isNewlyPlaced = other.isNewlyPlaced;
        this->towersIndex = other.towersIndex;
        sprite.setTexture(*texture);
        sprite.setPosition(gridX * TILE_SIZE, gridY * TILE_SIZE);
        sprite.setScale(float(TILE_SIZE) / texture->getSize().x, float(TILE_SIZE) / texture->getSize().x);

        attackRadiusShape.setRadius(attackRadius);
        attackRadiusShape.setOrigin(attackRadius, attackRadius);
        attackRadiusShape.setFillColor(sf::Color(255, 0, 0, 50));
        attackRadiusShape.setPosition(sprite.getPosition());
    }
    return *this;
}

void _Tower::setBulletTexture(const sf::Texture& texture) {
    bulletTexture = texture;
}

void _Tower::update(std::vector<Enemy>& enemies, float deltaTime) {
    if (attackClock.getElapsedTime().asSeconds() >= attackCooldown) {
        for (Enemy& e : enemies) {
            if (e.Dead()) continue;

            sf::Vector2f towerCenter = sprite.getPosition() + sf::Vector2f(TILE_SIZE / 2, TILE_SIZE / 2);
            sf::Vector2f enemyPos = e.getPosition();

            float dx = enemyPos.x - towerCenter.x;
            float dy = enemyPos.y - towerCenter.y;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance <= attackRadius) {
                bullets.emplace_back(50 + towersIndex*60, 300.f, bulletTexture);
                bullets.back().shoot(towerCenter, &e);
                attackClock.restart();
                break;
            }
        }
    }

    // Cập nhật đạn với deltaTime thực
    for (Bullet& b : bullets) {
        b.update(deltaTime);
    }

    // Xoá đạn không còn active
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [](const Bullet& b) { return !b.isActive(); }), bullets.end());
}




void _Tower::draw(sf::RenderWindow& window) {
    //window.draw(sprite);
    if (isNewlyPlaced) {
        sf::Sprite scaledSprite = sprite;
        float scale = 0.5f;
        scaledSprite.setScale(scale, scale);

        float offset = TILE_SIZE * (scale - 1.f) / 2.f;
        float moveUp = TILE_SIZE * 1.7f;    // ← lệch lên trên
        float moveLeft = TILE_SIZE * 0.75f;  // ← lệch sang trái 50% tile
        float moveDown = TILE_SIZE * 0.3f; // ← lệch xuong
        scaledSprite.setPosition(
            sprite.getPosition().x - offset - moveLeft,
            sprite.getPosition().y - offset - moveUp + moveDown
        );

        window.draw(scaledSprite);
    }
    else {
        window.draw(sprite);
    }
    /*window.draw(attackRadiusShape);*/
    // hiển thị vùng bắn
    for (auto& b : bullets) {
        b.draw(window);
    }
}

_Tower::_Tower(int gridX, int gridY, float radius, float cooldown, const sf::Texture* texture, bool fromSave, int towerIdx)
    : gridX(gridX), gridY(gridY), attackRadius(radius), attackCooldown(cooldown), texture(texture), towersIndex(towerIdx)
{
    sprite.setTexture(*texture);

    // Scale y như lúc hiển thị chính thức trong draw()
    float targetSize = TILE_SIZE * 0.9f; // giống draw logic
    float scaleX = targetSize / texture->getSize().x;
    float scaleY = targetSize / texture->getSize().y;
    sprite.setScale(scaleX, scaleY);

    sprite.setOrigin(texture->getSize().x / 2.f, texture->getSize().y / 2.f);
    sprite.setPosition(gridX * TILE_SIZE + TILE_SIZE / 2.f, gridY * TILE_SIZE + TILE_SIZE / 2.f);

    // Tắt hiệu ứng scale nhỏ sau khi Load
    isNewlyPlaced = fromSave;

    // Cập nhật vùng tấn công
    attackRadiusShape.setRadius(attackRadius);
    attackRadiusShape.setOrigin(attackRadius, attackRadius);
    attackRadiusShape.setFillColor(sf::Color(255, 0, 0, 50));
    attackRadiusShape.setPosition(sprite.getPosition());
}

void _Tower::setPosition(int x, int y) {
    gridX = x;
    gridY = y;
    sprite.setPosition(x * TILE_SIZE, y * TILE_SIZE);
    attackRadiusShape.setPosition(x * TILE_SIZE + TILE_SIZE / 2, y * TILE_SIZE + TILE_SIZE / 2);
}

Cpoint _Tower::getPosition() const {
    return Cpoint(gridX, gridY);
}

float _Tower::getAttackRadius() const {
    return attackRadius;
}

void _Tower::setTowersIndex(int i)
{
    towersIndex = i;
}

int _Tower::getIndex() const
{
    return towersIndex;
}
