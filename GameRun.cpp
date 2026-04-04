#include "gamerun.h"
#include "Library.h"
#include "gamelogic.h"
#include "SoundManager.h"
#include "VolumeSlider.h"
#include <iostream>
using namespace std;

bool Reset = false;
bool showTowers = true;
int lastBuildX, lastBuildY;
std::vector<sf::Texture> towerTextures;
std::vector<sf::Texture> bulletTextures;
int towerIndex = 0;

SoundManager sound;
VolumeSlider* volumeSlider = nullptr;

VolumeSlider* menuSlider = nullptr;
VolumeSlider* gameSlider = nullptr;
sf::Texture menuIconTexture, gameIconTexture;
sf::Sprite menuIcon, gameIcon;

enum class SaveState { None, Saving, Done };
SaveState saveState = SaveState::None;
sf::Clock saveClock;
bool saveValid = true;
float totalPlayTime = 0.f;
sf::Clock playClock;
std::vector<std::string> saveFiles;
std::string selectedFileName;
bool saveNameInputMode = false;
std::string saveFileName = "";
std::stack<std::string> mapHistory;
std::vector<sf::FloatRect> deleteButtons;
std::vector<sf::FloatRect> renameButtons;
bool renameInputMode = false;
std::string renameFileName;
int renameFileIndex = -1;
bool showSavingStatus = false;
float savingTimer = 0.f;

GameRun::GameRun()
    : currentMapName("menu"), savedMapName(""), currentMapIndex(0), isGamePaused(false),
    spawnInterval(2.5f - currentMapIndex * 0.2), totalEnemiesToSpawn(50), spawnedEnemies(0),
    radius(150.f), cooldown(1.0f),
    mapFiles({ "maps/map1.txt", "maps/map2.txt", "maps/map3.txt", "maps/map4.txt" })
{
    loadMapAndTileset(currentMapName);
    window.create(sf::VideoMode(gameMap.getWidth() * TILE_SIZE, gameMap.getHeight() * TILE_SIZE), "Tower Defense");
    if (!sound.load()) std::cerr << "Không thể load âm thanh.\n";
    sound.playMenuMusic();
    menuSlider = new VolumeSlider(&sound.getMenuVolumeRef());
    menuSlider->setPosition(350.f, 200.f);

    gameSlider = new VolumeSlider(&sound.getGameVolumeRef());
    gameSlider->setPosition(500.f, 200.f);


    menuIconTexture.loadFromFile("Audio/menu_icon.png");
    gameIconTexture.loadFromFile("Audio/game_icon.png");

    menuIcon.setTexture(menuIconTexture);
    gameIcon.setTexture(gameIconTexture);

    menuIcon.setScale(1.5, 1.5);
    gameIcon.setScale(1.5, 1.5);
    gold.setGold(1000);
}

void GameRun::run() {
    while (window.isOpen()) {
        processEvents();
        float dt = deltaClock.restart().asSeconds();
        update(dt);
        render();
    }
}

void GameRun::shutdownAudio() {
    sound.stopMusic();
    delete volumeSlider;
    volumeSlider = nullptr;
    menuSlider = nullptr;
    gameSlider = nullptr;
}

void GameRun::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::M) {
            sound.toggleMute();
        }

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sound.playClick();
            handleMouseClick(event.mouseButton.x, event.mouseButton.y);
        }

        if (currentMapName == "settings") {
            if (menuSlider) menuSlider->handleEvent(event, window);
            if (gameSlider) gameSlider->handleEvent(event, window);
        }

        if (event.type == sf::Event::TextEntered) {
            if (renameInputMode && !saveNameInputMode) {
                if (event.text.unicode == '\b') {
                    if (!renameFileName.empty())
                        renameFileName.pop_back();
                }
                else if (event.text.unicode == '\r' || event.text.unicode == '\n') {
                    std::string oldPath = "Saves/" + saveFiles[renameFileIndex];
                    std::string newPath = "Saves/" + renameFileName + ".txt";

                    std::ifstream checkExist(newPath);
                    if (!checkExist.good()) {
                        std::rename(oldPath.c_str(), newPath.c_str());
                        refreshSaveFiles();
                    }

                    renameInputMode = false;
                    renameFileName.clear();
                    renameFileIndex = -1;
                }
                else if (event.text.unicode >= 32 && event.text.unicode < 127 && renameFileName.size() < 20) {
                    renameFileName += static_cast<char>(event.text.unicode);
                }
            }

            else if (saveNameInputMode && !renameInputMode) {
                if (event.text.unicode == '\b') {
                    if (!saveFileName.empty())
                        saveFileName.pop_back();
                }
                else if (event.text.unicode == '\r' || event.text.unicode == '\n') {
                    std::string savePath = "Saves/" + saveFileName + ".txt";
                    std::ofstream test(savePath);
                    if (test.good()) {
                        test.close();
                        saveGameToFile(savePath);
                        refreshSaveFiles();
                    }

                    saveNameInputMode = false;
                    saveFileName.clear();
                    saveState = SaveState::Saving;
                    saveClock.restart();
                }
                else if (event.text.unicode >= 32 && event.text.unicode < 127 && saveFileName.size() < 20) {
                    saveFileName += static_cast<char>(event.text.unicode);
                }
            }
        }
        if (saveNameInputMode && event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                saveNameInputMode = false;
                saveFileName.clear();
                saveState = SaveState::None;

                if (!mapHistory.empty()) {
                    std::string backTo = mapHistory.top();
                    mapHistory.pop();
                    handleMapSwitch(backTo); // ✅ đúng cách!
                }
            }
        }
        if (renameInputMode && event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                renameInputMode = false;
                renameFileName.clear();
                renameFileIndex = -1;
            }
        }
    }
}

bool GameRun::checkEndGameCondition() {
    for (const auto& e : enemies) {
        if (e.reachedGoal()) {
            savedMapName = currentMapName;
            handleMapSwitch("defeat");
            return true;
        }
    }

    if (spawnedEnemies >= totalEnemiesToSpawn) {
        bool allDead = std::all_of(enemies.begin(), enemies.end(), [](const Enemy& e) {
            return e.Dead();
            });
        if (allDead) {
            savedMapName = currentMapName;
            handleMapSwitch("victory");
            return true;
        }
    }

    return false;
}

void GameRun::saveGameToFile(const std::string& filename)
{
    std::ofstream out(filename);
    if (!out) return;
    std::string realMapName =
        (currentMapName == "pause" || currentMapName == "save") && !savedMapName.empty()
        ? savedMapName
        : currentMapName;


    if (realMapName != "map1" && realMapName != "map2" &&
        realMapName != "map3" && realMapName != "map4") {
        saveValid = false;
        return;
    }
    saveValid = true;
    std::time_t now = std::time(nullptr);

    out << savedMapName << "\n";
    out << now << "\n";
    out << spawnedEnemies << "\n";
    out << towers.size() << "\n";
    out << gold.getGold() << "\n";
    out << static_cast<int>(totalPlayTime) << "\n";


    for (const auto& tower : towers) {

        out << "T " << tower.getPosition().get_cpointX() << " "
            << tower.getPosition().get_cpointY() << " "
            << tower.getIndex() << "\n";
    }

    for (const auto& e : enemies) {
        if (!e.Dead()) {
            out << "E " << e.getTextureFolder() << " " << e.getHealth()
                << " " << e.getPosition().x << " " << e.getPosition().y
                << " " << e.getCurrentTargetIndex()
                << " " << e.getSpawnX() << " " << e.getSpawnY() << " " << e.getSpeed();
            if (savedMapName == "map4") {
                // Ước lượng hướng spawn từ vị trí và điểm kế tiếp (nếu có)
                auto path = e.extractPathFromMap(mapFiles[currentMapIndex], Cpoint(e.getSpawnX(), e.getSpawnY()));
                if (path.size() >= 2) {
                    int dirX = path[1].get_cpointX() - path[0].get_cpointX();
                    int dirY = path[1].get_cpointY() - path[0].get_cpointY();
                    out << " " << dirX << " " << dirY;
                }
            }
            out << "\n";
        }
    }
}

void GameRun::loadGameFromFile(const std::string& filename)
{
    std::ifstream in(filename);
    if (!in) {
        return;
    }

    std::string mapName;
    std::time_t timestamp;
    int towerCount, totalTime, goldAmount;
    in >> mapName >> timestamp >> spawnedEnemies >> towerCount >> goldAmount >> totalTime;
    if (mapName == "map1") currentMapIndex = 0;
    else if (mapName == "map2") currentMapIndex = 1;
    else if (mapName == "map3") currentMapIndex = 2;
    else if (mapName == "map4") currentMapIndex = 3;
    gold.setGold(goldAmount);
    totalPlayTime = static_cast<float>(totalTime);
    playClock.restart();

    if (!loadMapAndTileset(mapName)) {
        return;
    }

    currentMapName = mapName;
    savedMapName = mapName;
    isGamePaused = false;
    Reset = false;
    showTowers = true;


    towers.clear();
    enemies.clear();

    std::string type;
    while (in >> type) {
        if (type == "T") {
            int x, y, towerIdx;
            in >> x >> y >> towerIdx;

            if (towerIdx >= 0 && towerIdx < towerTextures.size()) {
                _Tower tower(x, y, radius, cooldown, &towerTextures[towerIdx], true, towerIdx);
                tower.setBulletTexture(bulletTextures[towerIdx]);
                tower.setTowersIndex(towerIdx);
                towers.push_back(tower);
            }
        }
        else if (type == "E") {
            std::string folder;
            int hp, target, spawnX, spawnY;
            float x, y, speed;
            in >> folder >> hp >> x >> y >> target >> spawnX >> spawnY >> speed;

            if (mapName == "map1" || mapName == "map2" || mapName == "map3") {
                Enemy e(mapFiles[currentMapIndex], folder, speed, currentMapIndex + 1);
                e.setHealth(hp);
                e.setPosition({ x, y });
                e.setCurrentTargetIndex(target);
                enemies.push_back(e);
            }
            else if (mapName == "map4") {
                int dirX = 0, dirY = 0;
                if (!(in >> dirX >> dirY)) {
                    dirX = 1; dirY = 0; // fallback hướng phải nếu thiếu
                }
                sf::Vector2i spawn(spawnX, spawnY);
                Enemy e(mapFiles[currentMapIndex], folder, speed, currentMapIndex + 1, spawn);
                auto path = Enemy::extractPathFromMap(mapFiles[currentMapIndex], Cpoint(spawnX, spawnY), Cpoint(dirX, dirY));
                e.setPath(path);
                e.setHealth(hp);
                e.setPosition({ x, y });
                e.setCurrentTargetIndex(target);
                enemies.push_back(e);
            }

        }
    }
}

void GameRun::refreshSaveFiles() {
    saveFiles.clear();

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA("Saves\\*.txt", &findData);

    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do {
        saveFiles.push_back(std::string(findData.cFileName));
    } while (FindNextFileA(hFind, &findData) != 0);

    FindClose(hFind);
}

void GameRun::goToMap(const std::string& mapName, bool reset)
{
    Reset = reset;
    handleMapSwitch(mapName);
}


