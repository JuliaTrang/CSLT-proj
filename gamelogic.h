#pragma once
#include <vector>
#include "Tile.h"
//hàm đặt trụ súng tower bằng cách nhấn vào ô buildable
void handleClick(int x, int y, std::vector<std::vector<TileType>>& mapData, int TILE_SIZE, int width, int height);