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

void GameRun::handleMouseClick(int x, int y) {
    int tileX = x / TILE_SIZE;
    int tileY = y / TILE_SIZE;
    TileType clicked = gameMap.getTileTypeAt(tileX, tileY);

    bool isMapSwitch = false;
    std::vector<int> towerCosts = { 300, 500, 700, 900 };

    switch (clicked)
    {
    case Play:      mapHistory.push(currentMapName); isMapSwitch = true; currentMapName = "newgame"; break;
    case Tutorial:      mapHistory.push(currentMapName); isMapSwitch = true; currentMapName = "tutorial"; break;
    case Information:      mapHistory.push(currentMapName); isMapSwitch = true; currentMapName = "tutorial"; break;
    case Newgame:      mapHistory.push(currentMapName); isMapSwitch = true; Reset = true; currentMapName = "maps"; break;
    case Settings:  mapHistory.push(currentMapName); isMapSwitch = true; currentMapName = "settings"; break;
    case Load:      mapHistory.push(currentMapName); refreshSaveFiles(); isMapSwitch = true; currentMapName = "load"; break;
    case Save:
        isMapSwitch = true;
        mapHistory.push(currentMapName);
        /*currentMapName = "save";*/
        handleMapSwitch("save");
        saveNameInputMode = true;
        saveFileName.clear();
        renameInputMode = false;
        renameFileName.clear();
        renameFileIndex = -1;
        saveState = SaveState::None;
        break;
    case Quit:      isMapSwitch = true; mapHistory.push(currentMapName); currentMapName = "quit"; break;
    case Map1:      currentMapIndex = 0; isMapSwitch = true; mapHistory.push(currentMapName); currentMapName = "map1"; Reset = true; break;
    case Map2:      currentMapIndex = 1; isMapSwitch = true; mapHistory.push(currentMapName); currentMapName = "map2"; Reset = true; break;
    case Map3:      currentMapIndex = 2; isMapSwitch = true; mapHistory.push(currentMapName); currentMapName = "map3"; Reset = true; break;
    case Map4:      currentMapIndex = 3; isMapSwitch = true; mapHistory.push(currentMapName); currentMapName = "map4"; Reset = true; break;
    case Exit:      mapHistory.push(currentMapName); currentMapName = "menu"; isMapSwitch = true; isGamePaused = false; Reset = true; break;
    case Yes:       window.close(); return;
    case No:        mapHistory.push(currentMapName); currentMapName = "menu"; isMapSwitch = true; break;
    case Pause:
        mapHistory.push(currentMapName);
        savedMapName = currentMapName;
        currentMapName = "pause";
        isMapSwitch = true;
        isGamePaused = true;
        Reset = false;
        showTowers = false;
        break;
    case Back:

        if (saveNameInputMode) {
            saveNameInputMode = false;
            saveFileName.clear();
            currentMapName = mapHistory.empty() ? "menu" : mapHistory.top();
            if (!mapHistory.empty()) mapHistory.pop();
            /*std::string backTo = mapHistory.empty() ? "menu" : mapHistory.top();
            if (!mapHistory.empty()) mapHistory.pop();
            goToMap(backTo, false);*/
            isMapSwitch = true;
            if (currentMapName == "map1" || currentMapName == "map2" || currentMapName == "map3" || currentMapName == "map4") {
                sound.playGameMusic();
            }
            else {
                sound.playMenuMusic();
            }
            break;
        }
        if (currentMapName == "tower" && tempOldTower != nullptr) {
            towers.push_back(*tempOldTower);
            delete tempOldTower;
            tempOldTower = nullptr;

            // Đặt lại tile là Buildable để trụ xuất hiện đúng
            gameMap.setTileTypeAt(lastBuildX, lastBuildY, Buildable);
        }
        if (!mapHistory.empty()) {
            currentMapName = mapHistory.top();
            mapHistory.pop();
        }
        else {
            currentMapName = "menu"; // fallback
        }

        isMapSwitch = true;
        isGamePaused = false;
        Reset = false;
        showTowers = true;
        break;

    case Resume:
        if (!savedMapName.empty()) {
            if (!loadMapAndTileset(savedMapName)) {
                std::cerr << "Resume thất bại: không load được map " << savedMapName << "\n";
                break;
            }
            loadTowerAndBulletTextures(currentMapIndex);
            mapHistory.push(currentMapName);
            handleMapSwitch(savedMapName);
            isGamePaused = false;
            Reset = false;
            showTowers = true;
        }
        break;
    case Restart:
        if (!savedMapName.empty()) {
            mapHistory.push(currentMapName);
            currentMapName = savedMapName;
            isMapSwitch = true;
            isGamePaused = false;
            Reset = true;
        }
        break;

    case Buildable:

        for (auto it = towers.begin(); it != towers.end(); ++it) {
            if (it->getPosition().get_cpointX() == tileX &&
                it->getPosition().get_cpointY() == tileY)
            {

                if (tempOldTower != nullptr) {
                    delete tempOldTower;
                    tempOldTower = nullptr;
                }
                tempOldTower = new _Tower(*it);

                towers.erase(it);
                break;
            }
        }
        mapHistory.push(currentMapName);
        isGamePaused = true;
        showTowers = true;
        savedMapName = currentMapName;
        currentMapName = "tower";
        isMapSwitch = true;
        Reset = false;
        lastBuildX = tileX;
        lastBuildY = tileY;
        gameMap.setTileTypeAt(tileX, tileY, Grass);
        break;
    case choosing1:
        towerIndex = 0;
        {
            int cost = towerCosts[towerIndex];
            if (!gold.spendGold(cost)) {
                sound.playError(); return;
            }
            if (towerIndex < towerTextures.size()) {
                _Tower tower(lastBuildX, lastBuildY, radius, cooldown, &towerTextures[towerIndex]);
                tower.setBulletTexture(bulletTextures[towerIndex]);
                tower.isNewlyPlaced = true;
                tower.setTowersIndex(towerIndex);
                towers.push_back(tower);
            }
        }
        mapHistory.push(currentMapName);
        isGamePaused = false;
        currentMapName = savedMapName;
        gameMap.setTileTypeAt(lastBuildX, lastBuildY, Grass);
        isMapSwitch = true;
        Reset = false;
        break;

    case choosing2:
        towerIndex = 1;
        {
            int cost = towerCosts[towerIndex];
            if (!gold.spendGold(cost)) {
                sound.playError(); return;
            }
            if (towerIndex < towerTextures.size()) {
                _Tower tower(lastBuildX, lastBuildY, radius, cooldown, &towerTextures[towerIndex]);
                tower.setBulletTexture(bulletTextures[towerIndex]);
                tower.isNewlyPlaced = true;
                tower.setTowersIndex(towerIndex);
                towers.push_back(tower);
            }
        }
        mapHistory.push(currentMapName);
        isGamePaused = false;
        currentMapName = savedMapName;
        gameMap.setTileTypeAt(lastBuildX, lastBuildY, Grass);
        isMapSwitch = true;
        Reset = false;
        break;

    case choosing3:
        towerIndex = 2;
        {
            int cost = towerCosts[towerIndex];
            if (!gold.spendGold(cost)) {
                sound.playError(); return;
            }
            if (towerIndex < towerTextures.size()) {
                _Tower tower(lastBuildX, lastBuildY, radius, cooldown, &towerTextures[towerIndex]);
                tower.setBulletTexture(bulletTextures[towerIndex]);
                tower.isNewlyPlaced = true;
                tower.setTowersIndex(towerIndex);
                towers.push_back(tower);
            }
        }
        mapHistory.push(currentMapName);
        isGamePaused = false;
        currentMapName = savedMapName;
        gameMap.setTileTypeAt(lastBuildX, lastBuildY, Grass);
        isMapSwitch = true;
        Reset = false;
        break;

    case choosing4:
        towerIndex = 3;
        {
            int cost = towerCosts[towerIndex];
            if (!gold.spendGold(cost)) {
                sound.playError(); return;
            }
            if (towerIndex < towerTextures.size()) {
                _Tower tower(lastBuildX, lastBuildY, radius, cooldown, &towerTextures[towerIndex]);
                tower.setBulletTexture(bulletTextures[towerIndex]);
                tower.isNewlyPlaced = true;
                tower.setTowersIndex(towerIndex);
                towers.push_back(tower);
            }
        }
        mapHistory.push(currentMapName);
        isGamePaused = false;
        currentMapName = savedMapName;
        gameMap.setTileTypeAt(lastBuildX, lastBuildY, Grass);
        isMapSwitch = true;
        Reset = false;
        break;

    default:
        handleClick(x, y, gameMap.getMapData(), TILE_SIZE, gameMap.getWidth(), gameMap.getHeight());
        break;
    }

    if (isMapSwitch) {
        handleMapSwitch(currentMapName);
    }
    if (currentMapName == "load") {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));


        for (size_t i = 0; i < deleteButtons.size(); ++i) {
            if (deleteButtons[i].contains(mousePos)) {
                std::string path = "Saves/" + saveFiles[i];
                std::remove(path.c_str());
                refreshSaveFiles();
                return;
            }
            else if (renameButtons[i].contains(mousePos)) {
                saveNameInputMode = false;
                saveFileName.clear();
                renameFileIndex = i;
                renameInputMode = true;
                renameFileName = saveFiles[i];
                return;
            }
        }


        float boxX = (window.getSize().x - 1000.f) / 2.f;
        float boxY = (window.getSize().y - 600.f) / 2.f;
        float startY = boxY + 40.f;
        float lineHeight = 50.f;

        for (size_t i = 0; i < saveFiles.size(); ++i) {
            float y = startY + i * lineHeight;
            sf::FloatRect rowBox(boxX, y, 1000.f, lineHeight);

            if (rowBox.contains(mousePos)) {
                std::string selectedFile = saveFiles[i];


                if (selectedFile.find("map1") != std::string::npos) currentMapIndex = 0;
                else if (selectedFile.find("map2") != std::string::npos) currentMapIndex = 1;
                else if (selectedFile.find("map3") != std::string::npos) currentMapIndex = 2;
                else if (selectedFile.find("map4") != std::string::npos) currentMapIndex = 3;


                towers.clear();
                enemies.clear();

                loadTowerAndBulletTextures(currentMapIndex);
                loadGameFromFile("Saves/" + selectedFile);

                isMapSwitch = false;
                currentMapName = savedMapName;
                handleMapSwitch(savedMapName);
                return;
            }
        }
    }
}



void GameRun::handleMapSwitch(const std::string& targetMap) {
    if (!loadMapAndTileset(targetMap)) {
        std::cerr << "Không thể load map: " << targetMap << "\n";
        return;
    }
    bool inGameMap = (targetMap == "map1" || targetMap == "map2" || targetMap == "map3" || targetMap == "map4");
    if (inGameMap) {
        if (Reset) {
            loadTowerAndBulletTextures(currentMapIndex);
            resetGameState();
            enemySpawnClock.restart();
            totalPlayTime = 0.f;
            playClock.restart();
        }
        sound.playGameMusic();
    }
    else {
        if (Reset) {
            resetGameState();
        }
        sound.playMenuMusic();
    }
    currentMapName = targetMap;
    showTowers = (targetMap == "map1" || targetMap == "map2" || targetMap == "map3" || targetMap == "map4");
    sound.playClick();
}

void GameRun::resetGameState() {
    towers.clear();
    enemies.clear();
    spawnedEnemies = 0;
    gold.setGold(1000);
}

void GameRun::update(float dt) {
    bool inGameMap = (
        currentMapName == "map1" ||
        currentMapName == "map2" ||
        currentMapName == "map3" ||
        currentMapName == "map4"
        );
    if (!isGamePaused && inGameMap) {
        totalPlayTime += dt;
        if (enemySpawnClock.getElapsedTime().asSeconds() >= spawnInterval) {
            spawnEnemy();
            enemySpawnClock.restart();
        }

        for (auto& e : enemies) e.update(dt);
        for (auto& t : towers) t.update(enemies, dt);

        checkEndGameCondition(); // <- Gọi hàm kiểm tra kết thúc

        // Xử lý enemy chết → cộng vàng + xoá
        for (auto it = enemies.begin(); it != enemies.end(); ) {
            if (it->Dead()) {
                gold.addGold(50);  // ✅ cộng 50 vàng mỗi enemy chết (có thể thay bằng getReward() nếu cần)
                it = enemies.erase(it);
            }
            else if (it->reachedGoal()) {
                it = enemies.erase(it);  // xoá nếu chạm đích
            }
            else {
                ++it;
            }
        }


    }
    sound.setVolume(sound.getVolume());
    sound.setMenuVolume(sound.getMenuVolumeRef());
    sound.setGameVolume(sound.getGameVolumeRef());
}

void GameRun::render() {
    window.clear();
    gameMap.draw(window);
    if (showTowers && currentMapName.find("map") == 0)
        for (auto& tower : towers) tower.draw(window);
    if (currentMapName.find("map") == 0)
        for (auto& e : enemies) e.draw(window);

    if (currentMapName == "tutorial")
    {
        sf::Font Font;
        Font.loadFromFile("Fonts/ariblk.ttf");


        sf::Text title("How to Play", Font, 70);
        title.setFillColor(sf::Color::Red);


        sf::FloatRect titleBounds = title.getLocalBounds();
        float titleX = (window.getSize().x - titleBounds.width) / 2.f;
        float titleY = 80.f;

        title.setPosition(titleX, titleY);
        window.draw(title);

        std::vector<std::string> instructions = {
    "1. Click on a buildable tile to place a tower. Choose a tower based on its cost.",
    "    You can click again on an existing tower to upgrade it and increase its damage.",
    "2. Defeat all enemies to win the level. Each enemy you destroy will reward you with additional money.",
    "3. Do not let enemies reach the end of the path. If once get through, you will lose the game.",
    "4. You can save your current progress and money at any time using the save function.",
    "    This allows you to continue playing later from where you left off."
        };
        sf::Font font;
        font.loadFromFile("Fonts/ariblk.ttf");

        float lineSpacing = 50.f;
        float padding = 30.f;
        float boxWidth = 1800.f;
        float boxHeight = instructions.size() * lineSpacing + padding * 2.2f;

        sf::Vector2u winSize = window.getSize();
        float boxX = (winSize.x - boxWidth) / 2.f;
        float boxY = (winSize.y - boxHeight) / 2.f;

        // 🔳 Vẽ box nền
        sf::RectangleShape textBox;
        textBox.setSize(sf::Vector2f(boxWidth, boxHeight));
        textBox.setFillColor(sf::Color(0, 0, 0, 200));  // Màu đen mờ
        textBox.setOutlineThickness(2.f);
        textBox.setOutlineColor(sf::Color::White);
        textBox.setPosition(boxX, boxY);
        window.draw(textBox);

        // ✍️ Vẽ các dòng text
        for (size_t i = 0; i < instructions.size(); ++i) {
            sf::Text line;
            line.setFont(font);
            line.setString(instructions[i]);
            line.setCharacterSize(30);
            line.setFillColor(sf::Color::White);

            // Căn giữa dòng trong box
            float lineWidth = line.getLocalBounds().width;
            float textX = boxX + padding;
            float textY = boxY + padding + i * lineSpacing;

            line.setPosition(textX, textY);
            window.draw(line);
        }

    }

    if (currentMapName == "save")
    {
        sf::Vector2u winSize = window.getSize();
        sf::Vector2f boxSize(1000.f, 500.f);
        float boxX = (winSize.x - boxSize.x) / 2.f;
        float boxY = (winSize.y - boxSize.y) / 2.f;
        float centerX = boxX + boxSize.x / 2.f;

        sf::RectangleShape settingsBox;
        settingsBox.setSize(boxSize);
        settingsBox.setFillColor(sf::Color(0, 0, 0, 180));
        settingsBox.setOutlineThickness(2.f);
        settingsBox.setOutlineColor(sf::Color::White);
        settingsBox.setPosition(boxX, boxY);
        window.draw(settingsBox);

        sf::Font font;
        font.loadFromFile("Fonts/ariblk.ttf");
        if (!saveValid) {
            sf::Text invalidText("This is not a valid game file", font, 36);
            invalidText.setFillColor(sf::Color::White);
            invalidText.setPosition(centerX - invalidText.getLocalBounds().width / 2.f, boxY + 200.f);
            window.draw(invalidText);
        }
        // Nếu đang saving
        else if (saveState == SaveState::Saving) {
            // Vẽ vòng tròn xoay
            static sf::Texture loadingTexture;
            static bool loaded = false;
            static sf::Sprite spinner;
            static float rotation = 0.f;

            if (!loaded) {
                loadingTexture.loadFromFile("Icons/spinner.png"); // bạn cần chuẩn bị ảnh
                spinner.setTexture(loadingTexture);
                spinner.setOrigin(loadingTexture.getSize().x / 2.f, loadingTexture.getSize().y / 2.f);
                spinner.setScale(0.5f, 0.5f);
                loaded = true;
            }

            rotation += 200 * deltaClock.restart().asSeconds(); // Xoay nhanh
            spinner.setRotation(rotation);
            spinner.setPosition(centerX, boxY + 200.f);
            window.draw(spinner);

            sf::Text savingText("Saving ...", font, 36);
            savingText.setFillColor(sf::Color::White);
            savingText.setPosition(centerX - savingText.getLocalBounds().width / 2.f, boxY + 380.f);
            window.draw(savingText);

            // Sau 1 giây thì đổi sang trạng thái Done
            if (saveClock.getElapsedTime().asSeconds() >= 1.f) {
                saveState = SaveState::Done;
            }
        }
        else if (saveState == SaveState::Done) {
            // Vẽ tick xanh
            static sf::Texture checkTexture;
            static bool loaded = false;
            static sf::Sprite check;

            if (!loaded) {
                checkTexture.loadFromFile("Icons/check.png"); // bạn cần chuẩn bị ảnh
                check.setTexture(checkTexture);
                check.setOrigin(checkTexture.getSize().x / 2.f, checkTexture.getSize().y / 2.f);
                check.setScale(0.5f, 0.5f);
                loaded = true;
            }

            check.setPosition(centerX, boxY + 200.f);
            window.draw(check);

            sf::Text savedText("Saved", font, 36);
            savedText.setFillColor(sf::Color::Green);
            savedText.setPosition(centerX - savedText.getLocalBounds().width / 2.f, boxY + 300.f);
            window.draw(savedText);
        }
    }

    if (currentMapName == "load") {
        sf::Vector2u winSize = window.getSize();
        sf::Vector2f boxSize(1000.f, 600.f);
        float boxX = (winSize.x - boxSize.x) / 2.f;
        float boxY = (winSize.y - boxSize.y) / 2.f;

        // Vẽ nền đen
        sf::RectangleShape loadBox;
        loadBox.setSize(boxSize);
        loadBox.setFillColor(sf::Color(0, 0, 0, 180));
        loadBox.setOutlineThickness(2.f);
        loadBox.setOutlineColor(sf::Color::White);
        loadBox.setPosition(boxX, boxY);
        window.draw(loadBox);

        // Font
        sf::Font font;
        font.loadFromFile("Fonts/ariblk.ttf");

        float lineHeight = 50.f;
        float startY = boxY + 40.f;
        deleteButtons.clear();
        renameButtons.clear();
        for (size_t i = 0; i < saveFiles.size(); ++i) {
            std::string fileName = saveFiles[i];
            std::ifstream in("Saves/" + fileName);

            std::string mapName;
            std::time_t timestamp = 0;
            if (in) {
                in >> mapName >> timestamp;
            }


            float rowY = startY + i * lineHeight;
            sf::RectangleShape fileBox(sf::Vector2f(boxSize.x - 60.f, lineHeight - 10.f));
            fileBox.setFillColor(sf::Color(50, 50, 50, 200));
            fileBox.setOutlineColor(sf::Color::White);
            fileBox.setOutlineThickness(1.f);
            fileBox.setPosition(boxX + 30.f, rowY);
            window.draw(fileBox);


            sf::Text fileText(fileName, font, 24);
            fileText.setFillColor(sf::Color::White);
            fileText.setPosition(boxX + 40.f, rowY + 5.f);
            window.draw(fileText);


            char timeBuffer[26];
            std::string timeStr = "Unknown";
            if (timestamp != 0 && ctime_s(timeBuffer, sizeof(timeBuffer), &timestamp) == 0) {
                timeStr = std::string(timeBuffer);
                if (!timeStr.empty() && timeStr.back() == '\n')
                    timeStr.pop_back();
            }

            sf::Text timeText(timeStr, font, 20);
            timeText.setFillColor(sf::Color(180, 180, 180));
            timeText.setPosition(boxX + 600.f, rowY + 7.f);
            window.draw(timeText);

            sf::RectangleShape deleteButton(sf::Vector2f(25.f, 25.f));
            deleteButton.setFillColor(sf::Color(200, 50, 50));
            deleteButton.setPosition(boxX + 900.f, rowY + 5.f);
            window.draw(deleteButton);
            deleteButtons.push_back(deleteButton.getGlobalBounds());

            // Tạo nút Rename nhỏ
            sf::RectangleShape renameButton(sf::Vector2f(25.f, 25.f));
            renameButton.setFillColor(sf::Color(50, 100, 200));
            renameButton.setPosition(boxX + 940.f, rowY + 5.f);
            window.draw(renameButton);
            renameButtons.push_back(renameButton.getGlobalBounds());

            sf::Text delText("X", font, 18);
            delText.setPosition(deleteButton.getPosition().x + 5, deleteButton.getPosition().y - 2);
            window.draw(delText);

            sf::Text renText("R", font, 18);
            renText.setPosition(renameButton.getPosition().x + 4, renameButton.getPosition().y - 2);
            window.draw(renText);
        }
        if (renameInputMode) {
            sf::RectangleShape inputBox;
            inputBox.setSize(sf::Vector2f(400.f, 50.f));
            inputBox.setFillColor(sf::Color(0, 0, 0, 200));
            inputBox.setOutlineThickness(2.f);
            inputBox.setOutlineColor(sf::Color::White);

            // Vị trí hộp nhập giữa màn hình
            float centerX = window.getSize().x / 2.f - inputBox.getSize().x / 2.f;
            float centerY = window.getSize().y / 2.f - inputBox.getSize().y / 2.f;
            inputBox.setPosition(centerX, centerY);
            window.draw(inputBox);

            sf::Font font;
            font.loadFromFile("Fonts/ariblk.ttf"); // bạn thay bằng font của bạn

            sf::Text inputText(renameFileName + "_", font, 24);  // gợi ý thêm dấu _ nhấp nháy
            inputText.setFillColor(sf::Color::White);
            inputText.setPosition(centerX + 10.f, centerY + 10.f);
            window.draw(inputText);

            // Hướng dẫn
            sf::Text hint("Enter to confirm, ESC to cancel", font, 18);
            hint.setFillColor(sf::Color(180, 180, 180));
            hint.setPosition(centerX + 10.f, centerY + 60.f);
            window.draw(hint);
        }
    }

    if (currentMapName == "tower") {
        static sf::Font font;
        static sf::Texture moneyTex;
        static bool loaded = false;

        if (!loaded) {
            font.loadFromFile("Fonts/pixel.ttf");
            moneyTex.loadFromFile("TileSets/tower/money.png");
            loaded = true;
        }

        std::vector<int> towerCosts = { 300, 500, 700, 900 };

        std::vector<sf::Vector2i> boxTiles = {
            {6, 5},   // Trụ 111
            {13, 5},  // Trụ 222
            {6, 10},  // Trụ 333
            {13, 10}  // Trụ 444
        };

        std::vector<float> offsetX = {
            35.f,   // Box 1
            115.f,   // Box 2 (nằm xa hơn, cần dịch phải nhiều hơn)
            35.f,   // Box 3
            115.f    // Box 4
        };

        sf::Sprite coin;
        coin.setTexture(moneyTex);
        coin.setScale(0.1f, 0.1f);

        for (int i = 0; i < 4; ++i) {
            int tileX = boxTiles[i].x;
            int tileY = boxTiles[i].y;


            float boxX = tileX * TILE_SIZE + offsetX[i];
            float boxY = tileY * TILE_SIZE;

            sf::Vector2f boxSize(160.f, 60.f);
            sf::RectangleShape box(boxSize);
            box.setFillColor(sf::Color(0, 0, 0, 180));
            box.setOutlineThickness(2.f);
            box.setOutlineColor(sf::Color::White);
            box.setPosition(boxX, boxY);
            window.draw(box);


            sf::Text cost(std::to_string(towerCosts[i]), font, 36);
            cost.setFillColor(sf::Color::White);
            cost.setPosition(boxX + 20.f, boxY + 8.f);
            window.draw(cost);


            coin.setPosition(boxX + boxSize.x - 60.f, boxY + 6.f);
            window.draw(coin);
        }
    }

    if (currentMapName == "settings") {
        sf::Vector2u winSize = window.getSize();
        sf::Vector2f boxSize(1000.f, 500.f);
        float boxX = (winSize.x - boxSize.x) / 2.f;
        float boxY = (winSize.y - boxSize.y) / 2.f;
        float centerX = boxX + boxSize.x / 2.f;

        float sliderWidth = 350.f;
        float sliderY1 = boxY + 180.f;
        float sliderY2 = boxY + 280.f;

        // Cập nhật vị trí slider + icon theo box
        menuSlider->setPosition(centerX - sliderWidth / 2.f, sliderY1);
        gameSlider->setPosition(centerX - sliderWidth / 2.f, sliderY2);
        menuIcon.setPosition(centerX - sliderWidth / 2.f - 50.f, sliderY1 - 15.f);
        gameIcon.setPosition(centerX - sliderWidth / 2.f - 50.f, sliderY2 - 15.f);

        sf::RectangleShape settingsBox;
        settingsBox.setSize(boxSize);
        settingsBox.setFillColor(sf::Color(0, 0, 0, 180));
        settingsBox.setOutlineThickness(2.f);
        settingsBox.setOutlineColor(sf::Color::White);
        settingsBox.setPosition((winSize.x - boxSize.x) / 2.f, (winSize.y - boxSize.y) / 2.f);
        window.draw(settingsBox);

        // Icon + slider
        if (menuSlider) menuSlider->draw(window);
        if (gameSlider) gameSlider->draw(window);
        window.draw(menuIcon);
        window.draw(gameIcon);

        // Text labels
        sf::Font font;
        font.loadFromFile("Fonts/ariblk.ttf"); // cần font

        sf::Text menuText("Menu Volume", font, 24);
        menuText.setFillColor(sf::Color::White);
        menuText.setPosition(centerX - sliderWidth / 2.f - 250.f, sliderY1 - 5.f);

        window.draw(menuText);

        sf::Text gameText("Game Volume", font, 24);
        gameText.setFillColor(sf::Color::White);
        gameText.setPosition(centerX - sliderWidth / 2.f - 250.f, sliderY2 - 5.f);
        window.draw(gameText);

        sf::Text audioText("Audio Settings", font, 36);
        audioText.setFillColor(sf::Color::White);
        float titleX = boxX + (boxSize.x - audioText.getLocalBounds().width) / 2.f;
        float titleY = boxY + 30.f;

        audioText.setPosition(titleX, titleY);
        window.draw(audioText);

        sf::Text muteText("Press M to mute", font, 30);
        muteText.setFillColor(sf::Color::White);
        float muteX = boxX + (boxSize.x - muteText.getLocalBounds().width) / 2.f;
        float muteY = titleY + audioText.getLocalBounds().height + 10.f;

        muteText.setPosition(muteX, muteY);
        window.draw(muteText);
    }

    const float goldOffset = 400.f;

    if (currentMapName == "tower") {
        gold.setPosition(window.getSize().x - goldOffset, 15.f);
        gold.draw(window, true); // có hộp nền
    }
    else if (
        currentMapName == "map1" ||
        currentMapName == "map2" ||
        currentMapName == "map3" ||
        currentMapName == "map4"
        ) {
        gold.setPosition(window.getSize().x - goldOffset, 15.f);
        gold.draw(window, true);
    }

    if (saveNameInputMode) {
        sf::RectangleShape inputBox(sf::Vector2f(500.f, 50.f));
        inputBox.setFillColor(sf::Color(0, 0, 0, 200));
        inputBox.setOutlineThickness(2.f);
        inputBox.setOutlineColor(sf::Color::White);
        inputBox.setPosition((window.getSize().x - 500.f) / 2.f, window.getSize().y / 2.f - 25.f);
        window.draw(inputBox);

        sf::Font font;
        font.loadFromFile("Fonts/ariblk.ttf");
        sf::Text inputText(saveFileName, font, 24);
        inputText.setFillColor(sf::Color::White);
        inputText.setPosition(inputBox.getPosition().x + 10.f, inputBox.getPosition().y + 10.f);
        window.draw(inputText);

        sf::Text hint("Enter save file name and press Enter, ESC to cancel", font, 18);
        hint.setFillColor(sf::Color(150, 150, 150));
        hint.setPosition(inputBox.getPosition().x, inputBox.getPosition().y - 30.f);
        window.draw(hint);
    }

    window.display();
}

void GameRun::spawnEnemy() {
    if (spawnedEnemies + 2 <= totalEnemiesToSpawn && currentMapIndex == 3) {
        float speed1 = 50.f + static_cast<float>(rand() % 20);
        float speed2 = 50.f + static_cast<float>(rand() % 20);
        std::string folder = "Enemymovement/Map4";

        sf::Vector2i topSpawn(5, 0);
        sf::Vector2i leftSpawn(0, 3);


        // Dùng hướng ưu tiên: xuống cho top, phải cho trái
        auto topPath = Enemy::extractPathFromMap("maps/map4.txt", Cpoint(topSpawn.x, topSpawn.y), Cpoint(0, 1));  // ↓
        auto leftPath = Enemy::extractPathFromMap("maps/map4.txt", Cpoint(leftSpawn.x, leftSpawn.y), Cpoint(1, 0)); // →

        Enemy eTop("maps/map4.txt", folder, speed1, 4, topSpawn);
        Enemy eLeft("maps/map4.txt", folder, speed2, 4, leftSpawn);

        eTop.setPath(topPath);
        eLeft.setPath(leftPath);

        enemies.push_back(eTop);
        enemies.push_back(eLeft);
        spawnedEnemies += 2;
    }
    else if (spawnedEnemies < totalEnemiesToSpawn) {
        float speed = currentMapIndex * 15.f + 20.f + static_cast<float>(rand() % 20);;
        std::string folder = "Enemymovement/Map" + std::to_string(currentMapIndex + 1);
        enemies.emplace_back(mapFiles[currentMapIndex], folder, speed, currentMapIndex + 1);
        spawnedEnemies++;
    }
}



bool GameRun::loadMapAndTileset(const std::string& mapName) {
    if (!gameMap.loadMapFromFile("maps/" + mapName + ".txt")) return false;
    if (mapName == "menu") {
        std::unordered_set<TileType> needed = {
            Background, Play, Settings, Tutorial, Quit, towerdefense
        };
        if (!tileset.loadFromFolder("TileSets/" + mapName, needed))
            return false;
    }
    else if (mapName == "map1")
    {
        std::unordered_set<TileType> needed = {
            Buildable, Grass, Castle, house1, house2, Path, Pause, Tree, tower1, tower2, tower3, tower4, Information, bush1, bush2
        };
        if (!tileset.loadFromFolder("TileSets/" + mapName, needed))
            return false;
    }
    else if (mapName == "map2")
    {
        std::unordered_set<TileType> needed = {
            Buildable, Grass, Castle, Path, Pause, Tree, volcano1, volcano2, tower1, tower2, tower3, tower4, Information, bones1, bones2, bones3, bones4
        };
        if (!tileset.loadFromFolder("TileSets/" + mapName, needed))
            return false;
    }
    else if (mapName == "map3")
    {
        std::unordered_set<TileType> needed = {
            Buildable, Grass, Castle, Path, Pause, flower, Tree, frog, tower1, tower2, tower3, tower4, Information, shell1, shell2, coral1, coral2, crab, starfish, statue1, statue2
        };
        if (!tileset.loadFromFolder("TileSets/" + mapName, needed))
            return false;
    }
    else if (mapName == "map4")
    {
        std::unordered_set<TileType> needed = {
            Buildable, Grass, Castle, Path, Pause, Tree, scorpion, catus, pyramid, tower1, tower2, tower3, tower4, Information, lake, stone, catus2
        };
        if (!tileset.loadFromFolder("TileSets/" + mapName, needed))
            return false;
    }
    else if (mapName == "maps")
    {
        std::unordered_set<TileType> needed = {
            Background, Map1, Map2, Map3, Map4, Back
        };
        if (!tileset.loadFromFolder("TileSets/" + mapName, needed))
            return false;
    }
    else if (mapName == "pause")
    {
        std::unordered_set<TileType> needed = {
             Background, Exit, Restart, Resume, Save, Settings
        };
        if (!tileset.loadFromFolder("TileSets/" + mapName, needed))
            return false;
    }
    else if (mapName == "quit")
    {
        std::unordered_set<TileType> needed = {
             Background, Blank, No, Yes
        };
        if (!tileset.loadFromFolder("TileSets/" + mapName, needed))
            return false;
    }
    else if (mapName == "tower")
    {
        std::unordered_set<TileType> needed = {
             Background, choosing1, choosing2, choosing3, choosing4, Back
        };
        if (!tileset.loadFromFolder("TileSets/" + mapName, needed))
            return false;
    }
    else if (mapName == "defeat")
    {
        std::unordered_set<TileType> needed = {
             Background, Banner, Restart, Exit
        };
        if (!tileset.loadFromFolder("TileSets/" + mapName, needed))
            return false;
    }
    else if (mapName == "victory")
    {
        std::unordered_set<TileType> needed = {
             Background, Banner, Restart, Exit
        };
        if (!tileset.loadFromFolder("TileSets/" + mapName, needed))
            return false;
    }
    else if (mapName == "settings")
    {
        std::unordered_set<TileType> needed = {
             Background, Back
        };
        if (!tileset.loadFromFolder("TileSets/" + mapName, needed))
            return false;
    }
    else if (mapName == "save")
    {
        std::unordered_set<TileType> needed = {
             Background, Back
        };
        if (!tileset.loadFromFolder("TileSets/" + mapName, needed))
            return false;
    }
    else if (mapName == "load")
    {
        std::unordered_set<TileType> needed = {
             Background, Back
        };
        if (!tileset.loadFromFolder("TileSets/" + mapName, needed))
            return false;
    }
    else if (mapName == "newgame")
    {
        std::unordered_set<TileType> needed = {
             Background, Newgame, Load, Back
        };
        if (!tileset.loadFromFolder("TileSets/" + mapName, needed))
            return false;
    }
    else if (mapName == "tutorial")
    {
        std::unordered_set<TileType> needed = {
             Background, Back
        };
        if (!tileset.loadFromFolder("TileSets/" + mapName, needed))
            return false;
    }
    else {
        if (!tileset.loadFromFolder("TileSets/" + mapName)) // load tất cả
            return false;
    }
    gameMap.setTileSet(&tileset);
    return true;
}

void GameRun::loadTowerAndBulletTextures(int mapIndex) {
    towerTextures.clear();
    bulletTextures.clear();

    for (int i = 1; i <= 4; ++i) {
        sf::Texture towerTex, bulletTex;
        std::string towerPath = "Tower/tower" + std::to_string(i) + ".png";
        std::string bulletPath = "Bullet/bullet" + std::to_string(i) + ".png";

        if (!towerTex.loadFromFile(towerPath)) {
            std::cerr << "Không thể load tower: " << towerPath << "\n";
        }
        if (!bulletTex.loadFromFile(bulletPath)) {
            std::cerr << "Không thể load bullet: " << bulletPath << "\n";
        }

        towerTextures.push_back(towerTex);
        bulletTextures.push_back(bulletTex);
    }
}
