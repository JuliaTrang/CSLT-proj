#pragma once
#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include "map.h"
#include "tileset.h"

class Game {
private:
    Map gameMap;
    TileSet* tileset;

public:
    Game(TileSet* ts) : tileset(ts) {}

    bool loadGameMap(const std::string& filename) {
        if (!gameMap.loadMapFromFile(filename)) return false;
        gameMap.setTileSet(tileset);
        return true;
    }

    void draw(sf::RenderWindow& window) {
        gameMap.draw(window);
        // Vẽ thêm các thành phần game khác ở đây (trụ, địch, ...)
    }

    Map& getMap() { return gameMap; }

    // Thêm các hàm xử lý logic game như update, xử lý click, v.v.
};

#endif
