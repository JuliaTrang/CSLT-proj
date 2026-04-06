#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <unordered_set>
#include "tile.h"
#include "tileset.h"

class TileSet {
public:
    bool loadFromFolder(const std::string& folderPath);   // Load ảnh từ thư mục
    bool loadFromFolder(const std::string& folderPath, const std::unordered_set<TileType>& neededTiles);
    sf::Texture& getTexture(TileType type);                // Lấy texture theo loại ô

private:
    std::unordered_map<TileType, sf::Texture> textures;
};