#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "map.h"
#include "tileset.h"
#include "Enemy.h"
#include "Tower.h"
#include "Gold.h"
class GameRun {
public:
    GameRun();
    void run();
    void shutdownAudio();
private:
    void processEvents();
    void handleMouseClick(int x, int y);
    void update(float dt);
    void render();
    void spawnEnemy();
    bool loadMapAndTileset(const std::string& mapName);
    void loadTowerAndBulletTextures(int mapIndex);
    void resetGameState();
    void handleMapSwitch(const std::string& targetMap);
    bool checkEndGameCondition();

    void saveGameToFile(const std::string& filename);
    void loadGameFromFile(const std::string& filename);
    void refreshSaveFiles();
    void goToMap(const std::string& mapName, bool reset);
private:
    sf::RenderWindow window;
    sf::Clock deltaClock;
    sf::Clock enemySpawnClock;

    Map gameMap;
    TileSet tileset;
    std::vector<Enemy> enemies;
    std::vector<_Tower> towers;

    sf::Texture towerTexture;
    sf::Texture bulletTexture;

    std::string currentMapName;
    std::string savedMapName;
    int currentMapIndex;
    bool isGamePaused;

    float spawnInterval;
    int totalEnemiesToSpawn;
    int spawnedEnemies;
    float radius;
    float cooldown;

    std::vector<std::string> mapFiles;
    Gold gold;
    _Tower* tempOldTower = nullptr; // trụ cũ bị tạm xoá khi vào màn hình chọn trụ mới

};
