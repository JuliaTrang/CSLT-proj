#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "cpoint.h"
class Enemy {
private:
    sf::Sprite sprite;
    std::vector<std::shared_ptr<sf::Texture>> frames;
    int currentFrame = 0;
    float animationTimer = 0.f;
    float frameDuration = 0.1f;
    vector<Cpoint> path;
    int currentTargetIndex = 0;
    float moveSpeed = 0.f;
    bool goalReached = false;
    bool alive = true;
    int health = 100;
    void setGridPosition(int x, int y);
    void loadAnimationFrames(const std::string& folderPath, int mapOrder); // load các frame ảnh từ thư mục

    std::string textureFolder;
    sf::Vector2i spawnPos;
public:
    // Constructor tự đọc đường đi từ map và animation từ thư mục ảnh
    Enemy(const string& mapFile, const string& textureFolder, float speed, int mapOrder);
    Enemy(std::string mapFilePath, std::string folder, float speed, int mapId, sf::Vector2i spawnPos);
    Enemy(const std::string& mapFile, const std::string& textureFolder, float speed, int mapOrder,
        int health, sf::Vector2f position, int targetIndex);
    static vector<Cpoint> extractPathFromMap(const std::string& filename, const Cpoint& start, const Cpoint& preferredDir);
    void setPath(const std::vector<Cpoint>& newPath);

    Cpoint pos;
    void update(float deltaTime); // cập nhật vị trí và animation
    void draw(sf::RenderWindow& window) { if (alive) window.draw(sprite); }
    void takeDamage(int dmg);
    bool reachedGoal() const;
    bool Dead() const;
    float getX() const;
    float getY() const;
    sf::Vector2f getPosition() const;
    const sf::Sprite& getSprite() const;
    int getHealth() const;
    int getCurrentTargetIndex() const;
    const std::string& getTextureFolder() const;

    void setHealth(int hp);
    void setCurrentTargetIndex(int cri);
    void setPosition(const sf::Vector2f& pos);

    int getSpawnX() const { return spawnPos.x; }
    int getSpawnY() const { return spawnPos.y; }

    float getSpeed() const { return moveSpeed; }

    static vector<Cpoint> extractPathFromMap(const string& filename, const Cpoint& start);
    static vector<Cpoint> extractPathFromMap(const string& filename);
};