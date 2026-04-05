#include "Enemy.h"
#include "Tile.h" // dùng TILE_SIZE
#include <fstream>
#include <cmath>
#include <iostream>
#include "Map.h"
#include "cpoint.h"
using namespace std;
Enemy::Enemy(const string& mapFile, const string& textureFolder, float speed, int mapOrder)
    : moveSpeed(speed), currentTargetIndex(1), goalReached(false), alive(true), health(100 + mapOrder*20),
    currentFrame(0), animationTimer(0.f), frameDuration(0.1f), textureFolder(textureFolder)
{
    path = extractPathFromMap(mapFile);
    if (!path.empty()) {
        loadAnimationFrames(textureFolder, mapOrder);
        if (!frames.empty()) {
            sprite.setTexture(*frames[0]);  // ✅ dereference shared_ptr
        }
        setGridPosition(path[0].get_cpointX(), path[0].get_cpointY());
    }
    else {
        cout << "Không tìm thấy đường đi trong map: " << mapFile << "\n";
    }
}

Enemy::Enemy(std::string mapFilePath, std::string folder, float speed, int mapId, sf::Vector2i spawnPos)
    : moveSpeed(speed), currentTargetIndex(1), goalReached(false), alive(true), health(100 + 20*mapId),
    currentFrame(0), animationTimer(0.f), frameDuration(0.1f), textureFolder(folder)
{
    this->spawnPos = spawnPos;
    Cpoint spawnC(spawnPos.x, spawnPos.y);
    path = extractPathFromMap(mapFilePath, spawnC);

    if (!path.empty()) {
        loadAnimationFrames(folder, mapId);
        if (!frames.empty()) {
            sprite.setTexture(*frames[0]);  // ✅
        }
        setGridPosition(spawnPos.x, spawnPos.y);
        pos.set_cpointX(spawnPos.x);
        pos.set_cpointY(spawnPos.y);
    }
    else {
        cout << "Không tìm được path từ vị trí spawn trong " << mapFilePath << "\n";
    }
}

Enemy::Enemy(const std::string& mapFile, const std::string& textureFolder, float speed, int mapOrder,
    int health, sf::Vector2f position, int targetIndex)
    : moveSpeed(speed), textureFolder(textureFolder), health(health + mapOrder*20),
    currentTargetIndex(targetIndex)
{
    path = extractPathFromMap(mapFile);
    loadAnimationFrames(textureFolder, mapOrder);
    pos = Cpoint(position.x, position.y);
    sprite.setPosition(position);

    if (!frames.empty()) {
        sprite.setTexture(*frames[0]);
        sprite.setScale(0.8f, 0.8f);
        sprite.setOrigin(frames[0]->getSize().x / 2.f, frames[0]->getSize().y / 2.f);
    }
}


void Enemy::update(float deltaTime) {
    if (!alive || goalReached || currentTargetIndex >= path.size()) return;

    // Update animation
    animationTimer += deltaTime;
    if (animationTimer >= frameDuration && !frames.empty()) {
        animationTimer = 0.f;
        currentFrame = (currentFrame + 1) % frames.size();
        sprite.setTexture(*frames[currentFrame]);  // ✅ đúng
    }


    // Di chuyển
    sf::Vector2f currentPos = sprite.getPosition();
    sf::Vector2f targetPos(
        path[currentTargetIndex].get_cpointX() * TILE_SIZE,
        path[currentTargetIndex].get_cpointY() * TILE_SIZE
    );
    sf::Vector2f direction = targetPos - currentPos;

    float distance = sqrt(direction.x * direction.x + direction.y * direction.y);
    if (distance < 1.f) {
        setGridPosition(path[currentTargetIndex].get_cpointX(), path[currentTargetIndex].get_cpointY());
        currentTargetIndex++;
        if (currentTargetIndex >= path.size()) {
            goalReached = true;
        }
    }
    else {
        direction /= distance;
        sprite.move(direction * moveSpeed * deltaTime);
    }
}

void Enemy::takeDamage(int dmg) {
    health -= dmg;
    if (health <= 0) {
        alive = false;
    }
}

bool Enemy::reachedGoal() const {
    return goalReached;
}

void Enemy::loadAnimationFrames(const string& folderPath, int mapOrder) {
    frames.clear();
    for (int i = 1; i <= 6; ++i) {
        auto tex = std::make_shared<sf::Texture>();
        string path = folderPath + "/" + to_string(i) + ".png";
        if (!tex->loadFromFile(path)) {
            std::cout << "Không thể load frame: " << path << "\n";
            continue;
        }
        frames.push_back(tex);
    }
}


void Enemy::setGridPosition(int x, int y) {
    sprite.setPosition(static_cast<float>(x * TILE_SIZE), static_cast<float>(y * TILE_SIZE));
}

vector<Cpoint> Enemy::extractPathFromMap(const string& filename) {
    vector<string> map_lines;
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Không thể mở file map: " << filename << "\n";
        return {};
    }

    string line;
    while (getline(file, line)) {
        map_lines.push_back(line);
    }

    int rows = map_lines.size();
    int cols = 0;
    if (!map_lines.empty()) {
        cols = map_lines[0].size();
    }

    Cpoint start(-1, -1);
    for (int y = 0; y < rows && start.get_cpointX() == -1; ++y) {
        for (int x = 0; x < cols; ++x) {
            if (map_lines[y][x] == 'P') {
                start = Cpoint(x, y);
                break;
            }
        }
    }

    if (start.get_cpointX() == -1) {
        cout << "Không tìm thấy điểm bắt đầu trong file: " << filename << "\n";
        return {};
    }

    return extractPathFromMap(filename, start);  // dùng lại phiên bản mới
}

vector<Cpoint> Enemy::extractPathFromMap(const string& filename, const Cpoint& start) {
    vector<string> map_lines;
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Không thể mở file map: " << filename << "\n";
        return {};
    }

    string line;
    while (getline(file, line)) {
        map_lines.push_back(line);
    }

    int rows = map_lines.size();
    int cols = 0;
    if (!map_lines.empty()) {
        cols = map_lines[0].size();
    }

    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    vector<Cpoint> path;

    const vector<Cpoint> directions = {
        Cpoint(1, 0),  // phải
        Cpoint(-1, 0), // trái
        Cpoint(0, 1),  // xuống
        Cpoint(0, -1)  // lên
    };

    // ✅ Bắt đầu từ điểm spawn được truyền vào
    if (start.get_cpointX() < 0 || start.get_cpointX() >= cols ||
        start.get_cpointY() < 0 || start.get_cpointY() >= rows ||
        map_lines[start.get_cpointY()][start.get_cpointX()] != 'P') {
        cout << "Vị trí spawn không hợp lệ trong path map.\n";
        return {};
    }

    Cpoint current = start;
    path.push_back(current);
    visited[current.get_cpointY()][current.get_cpointX()] = true;

    while (true) {
        bool moved = false;

        for (const auto& dir : directions) {
            int next_x = current.get_cpointX() + dir.get_cpointX();
            int next_y = current.get_cpointY() + dir.get_cpointY();

            if (next_x >= 0 && next_x < cols && next_y >= 0 && next_y < rows &&
                !visited[next_y][next_x] && map_lines[next_y][next_x] == 'P') {
                Cpoint next(next_x, next_y);
                path.push_back(next);
                visited[next_y][next_x] = true;
                current = next;
                moved = true;
                break;
            }
        }

        if (!moved) break;
    }

    return path;
}

std::vector<Cpoint> Enemy::extractPathFromMap(const std::string& filename, const Cpoint& start, const Cpoint& preferredDir) {
    std::vector<std::string> map_lines;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Không thể mở file: " << filename << "\n";
        return {};
    }

    std::string line;
    while (std::getline(file, line)) {
        map_lines.push_back(line);
    }

    int rows = map_lines.size();
    int cols = map_lines.empty() ? 0 : map_lines[0].size();

    if (start.get_cpointX() < 0 || start.get_cpointX() >= cols ||
        start.get_cpointY() < 0 || start.get_cpointY() >= rows ||
        map_lines[start.get_cpointY()][start.get_cpointX()] != 'P') {
        std::cout << "Vị trí bắt đầu không hợp lệ\n";
        return {};
    }

    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    std::vector<Cpoint> path;

    // ✅ Chỉ định hướng hợp lệ theo hướng spawn
    std::vector<Cpoint> directions;

    if (preferredDir.get_cpointX() == 0 && preferredDir.get_cpointY() == 1) {
        // Spawn từ trên → chỉ đi xuống ↓ rồi rẽ phải →
        directions = {
            Cpoint(0, 1),  // ↓
            Cpoint(1, 0)   // →
        };
    }
    else if (preferredDir.get_cpointX() == 1 && preferredDir.get_cpointY() == 0) {
        // Spawn từ trái → chỉ đi sang phải → rồi rẽ xuống ↓
        directions = {
            Cpoint(1, 0),  // →
            Cpoint(0, 1)   // ↓
        };
    }
    else {
        // fallback an toàn
        directions = { preferredDir };
    }

    Cpoint current = start;
    visited[current.get_cpointY()][current.get_cpointX()] = true;
    path.push_back(current);

    while (true) {
        bool moved = false;

        for (const auto& dir : directions) {
            int nx = current.get_cpointX() + dir.get_cpointX();
            int ny = current.get_cpointY() + dir.get_cpointY();

            if (nx >= 0 && nx < cols && ny >= 0 && ny < rows &&
                !visited[ny][nx] && map_lines[ny][nx] == 'P') {
                Cpoint next(nx, ny);
                path.push_back(next);
                visited[ny][nx] = true;
                current = next;
                moved = true;
                break;
            }
        }

        if (!moved) break;
    }

    return path;
}


bool Enemy::Dead() const {
    return !alive;
}

float Enemy::getX() const { return sprite.getPosition().x; }
float Enemy::getY() const { return sprite.getPosition().y; }
sf::Vector2f Enemy::getPosition() const {
    return sprite.getPosition();
}
const sf::Sprite& Enemy::getSprite() const {
    return sprite;
}
void Enemy::setPath(const std::vector<Cpoint>& newPath) {
    path = newPath;
    currentTargetIndex = 1;  // bắt đầu từ mục tiêu kế tiếp
}

int Enemy::getHealth() const
{
    return health;
}

int Enemy::getCurrentTargetIndex() const
{
    return currentTargetIndex;
}

const std::string& Enemy::getTextureFolder() const
{
    return textureFolder;
}

void Enemy::setHealth(int hp)
{
    health = hp;
}

void Enemy::setCurrentTargetIndex(int cri)
{
    currentTargetIndex = cri;
}

void Enemy::setPosition(const sf::Vector2f& pos)
{
    sprite.setPosition(pos);   // cập nhật vị trí sprite
    this->pos.set_cpointX(pos.x);
    this->pos.set_cpointY(pos.y);
}