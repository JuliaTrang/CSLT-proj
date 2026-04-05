#include "tileset.h"
#include "map.h"
#include "tile.h"

#include <fstream>
#include <iostream>
#include <SFML/Graphics.hpp>
using namespace std;

Map::Map() {
    height = 0;
    width = 0;
    tileset = nullptr;
}

void Map::setTileSet(TileSet* ts) {
    tileset = ts;
}

bool Map::loadMapFromFile(const std::string& filename) {
    ifstream file(filename);
    if (!file.is_open()) return false;

    mapData.clear();
    string line;
    while (std::getline(file, line)) {
        std::vector<TileType> row;
        for (char c : line) {
            switch (c) {
            case 'G': row.push_back(Grass); break;
            case 'P': row.push_back(Path); break;
            case 'B': row.push_back(Buildable); break;
            case 'C': row.push_back(Castle); break;
            case 'W': row.push_back(Background); break;
            case 'Y': row.push_back(Play); break;
            case 'S': row.push_back(Settings); break;
            case 'L': row.push_back(Load); break;
            case 'Q': row.push_back(Quit); break;
            case 'A': row.push_back(Map1); break;
            case 'I': row.push_back(Map2); break;
            case 'E': row.push_back(Map3); break;
            case 'U': row.push_back(Map4); break;
            case 'X': row.push_back(Yes); break;
            case 'N': row.push_back(No); break;
            case 'H': row.push_back(Blank); break;
            case 'O': row.push_back(Pause); break;
            case 'Z': row.push_back(Save); break;
            case 'R': row.push_back(Resume); break;
            case 'D': row.push_back(Exit); break;
            case 'F': row.push_back(Restart); break;
            case 'K': row.push_back(Tree); break;
            case 'h': row.push_back(house1); break;
            case 'k': row.push_back(house2); break;
            case 'c': row.push_back(catus); break;
            case 's': row.push_back(scorpion); break;
            case 'v': row.push_back(volcano1); break;
			case 'd': row.push_back(volcano2); break;
            case 'f': row.push_back(frog); break;
            case 'l': row.push_back(flower); break;
            case 'V': row.push_back(Back); break;
            case '1': row.push_back(choosing1); break;
            case '2': row.push_back(choosing2); break;
            case '3': row.push_back(choosing3); break;
            case '4': row.push_back(choosing4); break;
            case 'M': row.push_back(towerdefense); break;
			case 'p': row.push_back(pyramid); break;
			case 'J': row.push_back(Banner); break;
            case 'T': row.push_back(Tutorial); break;
            case '6': row.push_back(Newgame); break;
            case '5': row.push_back(Information); break;
            case 'b': row.push_back(bush1); break;
            case 'j': row.push_back(bush2); break;
			case 'o': row.push_back(bones1); break;
			case 'q': row.push_back(bones2); break;
			case 'w': row.push_back(bones3); break;
			case 'e': row.push_back(bones4); break;
			case 'y': row.push_back(shell1); break;
			case 't': row.push_back(shell2); break;
			case 'g': row.push_back(starfish); break;
			case 'n': row.push_back(coral1); break;
			case 'r': row.push_back(coral2); break;
			case '7': row.push_back(crab); break;
			case 'z': row.push_back(statue1); break;
			case 'x': row.push_back(statue2); break;
			case 'a': row.push_back(lake); break;
			case 'm': row.push_back(stone); break;
            default: row.push_back(Grass); break;
            }
        }
        mapData.push_back(row);
    }

    height = mapData.size();
    width = height > 0 ? mapData[0].size() : 0;
    return !mapData.empty();
}


int Map::getWidth() const {
    return this->width;
}

int Map::getHeight() const {
    return this->height;
}

void Map::drawSingleTileIfFound(sf::RenderWindow& window, TileType type, float scaleX, float scaleY) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (mapData[y][x] == type) {
                sf::Sprite sprite;
                sprite.setTexture(tileset->getTexture(type));
                sprite.setScale(
                    TILE_SIZE * scaleX / sprite.getTexture()->getSize().x,
                    TILE_SIZE * scaleY / sprite.getTexture()->getSize().y
                );
                sprite.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                window.draw(sprite);
                return; // chỉ vẽ 1 lần đầu tiên thấy
            }
        }
    }
}

void Map::draw(sf::RenderWindow& window)
{
    if (!tileset) return;

    // Duyệt và vẽ từng ô theo texture
    sf::Sprite sprite; // Tạo 1 đối tượng Sprite dùng chung để vẽ từng ô
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            TileType type = mapData[y][x]; // xác định tọa độ (x,y)
            if (type == Castle || type == Buildable || type == Tree ||
                type == house1 || type == house2 || type == catus || type == scorpion ||
                type == volcano1 || type == volcano2 || type == frog || type == flower || type == pyramid || type == Pause || type == Information || 
                type == bush1 || type == bush2 || type == stone || type == lake || type == catus2 || type == starfish || 
                type == statue1 || type == statue2 || type == coral1 || type == coral2 || type == bones1 || type == bones2 ||
                type == bones3 || type == bones4 || type == shell1 || type == shell2 || type == crab) {
                type = Grass;
            }
            else if (type == Play || type == Settings || type == Load || type == Quit ||
                type == Map1 || type == Map2 || type == Map3 || type == Map4 ||
                type == Blank || type == Restart || type == Save || type == Resume ||
                type == Exit || type == Back || type == choosing1 || type == choosing2 ||
                type == choosing3 || type == choosing4 || type == towerdefense || type == Banner || type == Newgame || type == Tutorial) {
                type = Background;
            }
            else if (type == Yes || type == No) {
                type = Blank;
            }
            sprite.setTexture(tileset->getTexture(type)); // coi coi ở tọa độ đó là chữ gì để tải ảnh phù hợp
            sprite.setScale(
                TILE_SIZE / (float)sprite.getTexture()->getSize().x,
                TILE_SIZE / (float)sprite.getTexture()->getSize().y
            ); // hàm xét kích thước để ảnh phù hợp với ô 
            sprite.setPosition(x * TILE_SIZE, y * TILE_SIZE); // Đặt sprite vào vị trí (x, y) trong cửa sổ
            window.draw(sprite); // vẽ sprite
        }
    }
    int xCastle = -1, yCastle = -1;
    bool foundCastle = false;
    for (int y = 0; y < height && !foundCastle; ++y) {
        for (int x = 0; x < width && !foundCastle; ++x) {
            if (mapData[y][x] == Castle) {
                xCastle = x;
                yCastle = y;
                foundCastle = true;
            }
        }
    }

    // Vẽ Castle nếu tìm thấy
    if (foundCastle) {
        sf::Sprite spriteCastle;
        spriteCastle.setTexture(tileset->getTexture(Castle));
        spriteCastle.setScale(
            TILE_SIZE * 3 / (float)spriteCastle.getTexture()->getSize().x,
            TILE_SIZE * 3 / (float)spriteCastle.getTexture()->getSize().y
        );
        spriteCastle.setPosition(xCastle * TILE_SIZE, yCastle * TILE_SIZE);
        window.draw(spriteCastle);
    }

    int xBackground = -1, yBackground = -1;
    bool foundBackground = false;
    for (int y = 0; y < height && !foundBackground; ++y) {
        for (int x = 0; x < width && !foundBackground; ++x) {
            if (mapData[y][x] == Background) {
                xBackground = x;
                yBackground = y;
                foundBackground = true;
            }
        }
    }

    // Vẽ Background nếu tìm thấy
    if (foundBackground) {
        sf::Sprite spritebackground;
        spritebackground.setTexture(tileset->getTexture(Background));
        spritebackground.setScale(
            TILE_SIZE * (width + 1) / (float)spritebackground.getTexture()->getSize().x,
            TILE_SIZE * (height + 1) / (float)spritebackground.getTexture()->getSize().y
        );
        spritebackground.setPosition(xBackground * TILE_SIZE, yBackground * TILE_SIZE);
        window.draw(spritebackground);
    }



    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (mapData[y][x] == Tree) {
                sf::Sprite spriteTTree;
                spriteTTree.setTexture(tileset->getTexture(Tree));
                spriteTTree.setScale(
                    TILE_SIZE / (float)spriteTTree.getTexture()->getSize().x,
                    TILE_SIZE / (float)spriteTTree.getTexture()->getSize().y
                );
                spriteTTree.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                window.draw(spriteTTree);
            }
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (mapData[y][x] == Buildable) {
                sf::Sprite spriteBuild;
                spriteBuild.setTexture(tileset->getTexture(Buildable));
                spriteBuild.setScale(
                    TILE_SIZE / (float)spriteBuild.getTexture()->getSize().x,
                    TILE_SIZE / (float)spriteBuild.getTexture()->getSize().y
                );
                spriteBuild.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                window.draw(spriteBuild);
            }
        }
    }

    
  

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (mapData[y][x] == tower1) {
                sf::Sprite spriteTower;
                spriteTower.setTexture(tileset->getTexture(tower1));
                spriteTower.setScale(
                    TILE_SIZE / static_cast<float>(spriteTower.getTexture()->getSize().x),
                    TILE_SIZE / static_cast<float>(spriteTower.getTexture()->getSize().y)
                );
                spriteTower.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                window.draw(spriteTower);
            }
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (mapData[y][x] == tower2) {
                sf::Sprite spriteTower;
                spriteTower.setTexture(tileset->getTexture(tower2));
                spriteTower.setScale(
                    TILE_SIZE / static_cast<float>(spriteTower.getTexture()->getSize().x),
                    TILE_SIZE / static_cast<float>(spriteTower.getTexture()->getSize().y)
                );
                spriteTower.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                window.draw(spriteTower);
            }
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (mapData[y][x] == tower3) {
                sf::Sprite spriteTower;
                spriteTower.setTexture(tileset->getTexture(tower3));
                spriteTower.setScale(
                    TILE_SIZE / static_cast<float>(spriteTower.getTexture()->getSize().x),
                    TILE_SIZE / static_cast<float>(spriteTower.getTexture()->getSize().y)
                );
                spriteTower.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                window.draw(spriteTower);
            }
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (mapData[y][x] == tower4) {
                sf::Sprite spriteTower;
                spriteTower.setTexture(tileset->getTexture(tower4));
                spriteTower.setScale(
                    TILE_SIZE / static_cast<float>(spriteTower.getTexture()->getSize().x),
                    TILE_SIZE / static_cast<float>(spriteTower.getTexture()->getSize().y)
                );
                spriteTower.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                window.draw(spriteTower);
            }
        }
    }


    drawSingleTileIfFound(window, towerdefense, 15, 2);
    drawSingleTileIfFound(window, Play, 5, 2);
    drawSingleTileIfFound(window, Settings, 9, 2);
    drawSingleTileIfFound(window, Load, 5, 2);
    drawSingleTileIfFound(window, Quit, 5, 2);
    drawSingleTileIfFound(window, Blank, 15, 5);
    drawSingleTileIfFound(window, Yes, 3, 2);
    drawSingleTileIfFound(window, No, 3, 2);
    drawSingleTileIfFound(window, Map1, 5, 3);
    drawSingleTileIfFound(window, Map2, 5, 3);
    drawSingleTileIfFound(window, Map3, 5, 3);
    drawSingleTileIfFound(window, Map4, 5, 3);
    drawSingleTileIfFound(window, Save, 5, 2);
    drawSingleTileIfFound(window, Resume, 7, 2);
    drawSingleTileIfFound(window, Exit, 5, 2);
    drawSingleTileIfFound(window, Restart, 7, 2);
    drawSingleTileIfFound(window, Pause, 1, 1);
    drawSingleTileIfFound(window, Back, 1, 1);
    drawSingleTileIfFound(window, Information, 1, 1);
    drawSingleTileIfFound(window, Newgame, 7, 2);
    drawSingleTileIfFound(window, Tutorial, 7, 2);
    drawSingleTileIfFound(window, choosing1, 3, 4);
    drawSingleTileIfFound(window, choosing2, 3, 4);
    drawSingleTileIfFound(window, choosing3, 3, 4);
    drawSingleTileIfFound(window, choosing4, 3, 4);
	drawSingleTileIfFound(window, Banner, 5, 4);


    drawDecorations(window, stone, 1, 1, 1.0f, 1.0f);
    drawDecorations(window, bones1, 3, 2, 3.0f, 2.0f);
    drawDecorations(window, bones2, 2, 2, 2.0f, 2.0f);
    drawDecorations(window, bones3, 4, 2, 4.0f, 2.0f);
    drawDecorations(window, bones4, 3, 3, 3.0f, 3.0f);
    drawDecorations(window, shell1, 1, 1, 1.0f, 1.0f);
    drawDecorations(window, shell2, 1, 1, 1.0f, 1.0f);
    drawDecorations(window, starfish, 1, 1, 1.0f, 1.0f);
    drawDecorations(window, lake, 2, 2, 2.0f, 2.0f);
    drawDecorations(window, crab, 2, 2, 2.0f, 2.0f);
    drawDecorations(window, statue1, 1, 2, 1.0f, 2.0f);
    drawDecorations(window, statue2, 1, 2, 1.0f, 2.0f);
    drawDecorations(window, flower, 1, 1, 1.0f, 1.0f);
    drawDecorations(window, house1, 2, 2, 2.0f, 2.0f);
    drawDecorations(window, house2, 2, 2, 2.0f, 2.0f);
    drawDecorations(window, frog, 2, 2, 2.0f, 2.0f);
    drawDecorations(window, volcano1, 5, 3, 5.0f, 3.0f);
    drawDecorations(window, volcano2, 5, 3, 5.0f, 3.0f);
    drawDecorations(window, scorpion, 2, 2, 2.0f, 2.0f);
    drawDecorations(window, catus, 2, 2, 2.0f, 2.0f);
	drawDecorations(window, pyramid, 6, 3, 6.0f, 3.0f);
    drawDecorations(window, bush1, 1, 1, 1.0f, 1.0f);
    drawDecorations(window, bush2, 1, 1, 1.0f, 1.0f);
}

void Map::drawDecorations(sf::RenderWindow& window, TileType type, int sizeW, int sizeH, float scaleX, float scaleY) {
    std::vector<std::vector<bool>> drawn(height, std::vector<bool>(width, false));

    for (int y = 0; y <= height - sizeH; ++y) {
        for (int x = 0; x <= width - sizeW; ++x) {
            if (drawn[y][x]) continue;

            bool match = true;
            for (int dy = 0; dy < sizeH && match; ++dy) {
                for (int dx = 0; dx < sizeW && match; ++dx) {
                    if (mapData[y + dy][x + dx] != type || drawn[y + dy][x + dx])
                        match = false;
                }
            }

            if (match) {
                // Vẽ sprite tại góc trên trái
                sf::Sprite sprite;
                sprite.setTexture(tileset->getTexture(type));
                sprite.setScale(
                    TILE_SIZE * scaleX / sprite.getTexture()->getSize().x,
                    TILE_SIZE * scaleY / sprite.getTexture()->getSize().y
                );
                sprite.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                window.draw(sprite);

                // Đánh dấu đã vẽ để không vẽ lại
                for (int dy = 0; dy < sizeH; ++dy)
                    for (int dx = 0; dx < sizeW; ++dx)
                        drawn[y + dy][x + dx] = true;
            }
        }
    }
}

vector<std::vector<TileType>>& Map::getMapData() {
    return mapData;
}

TileType Map::getTileTypeAt(int x, int y) const {
    if (x < 0 || y < 0 || x >= width || y >= height) return Grass;
    return mapData[y][x];
}

void Map::setTileTypeAt(int x, int y, TileType type) {
    if (x >= 0 && x < width && y >= 0 && y < height)
        mapData[y][x] = type;
}

Cpoint Map::findTopSpawn() const {
    for (int x = 0; x < width; ++x) {
        if (mapData[0][x] == Path)
            return Cpoint(x, 0);
    }
    return Cpoint(-1, -1); // không tìm thấy
}

Cpoint Map::findLeftSpawn() const {
    for (int y = 0; y < height; ++y) {
        if (mapData[y][0] == Path)
            return Cpoint(0, y);
    }
    return Cpoint(-1, -1); // không tìm thấy
}
