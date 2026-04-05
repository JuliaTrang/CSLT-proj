#include "Bullet.h"
#include <cmath>
#include "Map.h"

Bullet::Bullet() : damage(50), speed(180.f), target(nullptr), active(false) {}

Bullet::Bullet(int dmg, float spd, const sf::Texture& tex)
    : damage(dmg), speed(spd), texture(tex), target(nullptr), active(false) {
    sprite.setTexture(texture);
    sprite.setScale(1.3f, 1.2f);
}


void Bullet::shoot(const sf::Vector2f& startPos, Enemy* targetEnemy) {
    sprite.setPosition(startPos);
    target = targetEnemy;
    aliveTime = 0.f;      // Reset thời gian tồn tại khi bắn mới
    active = true;
    // Tính hướng tới enemy
    sf::Vector2f targetPos = target->getPosition();
    sf::Vector2f direction = targetPos - startPos;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (length != 0) {
        velocity = direction / length * speed;
    }
    
}

void Bullet::update(float deltaTime) {
    if (!active || target == nullptr || target->Dead()) {
        active = false;
        return;
    }
    float maxAliveTime = 0.6f;
    aliveTime += deltaTime;  // Cộng dồn thời gian sống

    if (aliveTime > maxAliveTime) {
        // Đạn bay quá thời gian max mà chưa trúng target, tắt đạn
        active = false;
        target = nullptr;
        return;
    }

    sf::Vector2f oldPos = sprite.getPosition();
    sf::Vector2f moveVec = velocity * deltaTime;
    sf::Vector2f newPos = oldPos + moveVec;

    // Kiểm tra va chạm với target (giữ nguyên đoạn kiểm tra cũ của bạn)
    sf::Vector2f enemyPos = target->getPosition();
    float radius = 30.f;

    sf::Vector2f segVec = newPos - oldPos;
    sf::Vector2f ptVec = enemyPos - oldPos;

    float segLenSquared = segVec.x * segVec.x + segVec.y * segVec.y;
    float proj = (ptVec.x * segVec.x + ptVec.y * segVec.y) / segLenSquared;
    proj = std::max(0.f, std::min(1.f, proj));

    sf::Vector2f closestPoint = oldPos + segVec * proj;
    float dx = enemyPos.x - closestPoint.x;
    float dy = enemyPos.y - closestPoint.y;
    float distToEnemy = std::sqrt(dx * dx + dy * dy);

    if (distToEnemy < radius) {
        target->takeDamage(damage);
        active = false;
        target = nullptr;
        return;
    }

    sprite.setPosition(newPos);
}


void Bullet::draw(sf::RenderWindow& window) {
    if (active) {
        window.draw(sprite);
    }
}

bool Bullet::isActive() const {
    return active;
}

bool Bullet::hasHitTarget() const {
    return !active && target;
}

Enemy* Bullet::getTarget() const {
    return target;
}