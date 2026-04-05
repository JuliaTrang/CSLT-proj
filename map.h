#pragma once
#include <vector>
#include <string>
#include "tile.h"
#include "tileset.h"
#include "cpoint.h"
using namespace std;

const int TILE_SIZE = 80; // kích thước 1 ô 

class Map {
private:
	int width;
	int height;
	vector<std::vector<TileType>> mapData;
	TileSet* tileset;
public:
	Map();
	bool loadMapFromFile(const std::string& filename); // tải map lên từ file txt
	int getWidth() const;
	int getHeight() const;
	void draw(sf::RenderWindow& window);
	void setTileSet(TileSet* ts);
	vector<std::vector<TileType>>& getMapData();
	TileType getTileTypeAt(int x, int y) const;
	void setTileTypeAt(int x, int y, TileType type);
	void drawDecorations(sf::RenderWindow& window, TileType type, int sizeW, int sizeH, float scaleX, float scaleY);
	void drawSingleTileIfFound(sf::RenderWindow& window, TileType type, float scaleX, float scaleY);
	Cpoint findTopSpawn() const;
	Cpoint findLeftSpawn() const;

};