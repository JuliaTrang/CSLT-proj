#include "tileset.h"
#include <iostream>
using namespace std;

bool TileSet::loadFromFolder(const std::string& folderPath) {
    std::unordered_map<TileType, std::string> filenames = {
        {Grass, "grass.png"},
        {Path, "path.png"},
        {Buildable, "buildable.png"},
        {Castle, "castle.png"},
        {Background, "background.png"},
        {Play, "play.png"},
        {Settings, "settings.png"},
        {Load, "load.png"},
        {Quit, "quit.png"},
        {Map1, "map1.png"},
        {Map2, "map2.png"},
        {Map3, "map3.png"},
        {Map4, "map4.png"},
        {Blank, "blank.png"},
        {Yes, "yes.png"},
        {No, "no.png"},
        {Pause, "pause.png" },
        {Save, "save.png"  },
        {Resume, "resume.png" },
        {Exit, "exit.png" },
        {Restart, "restart.png" },
        {Tree, "tree.png"},
        {house1, "house1.png"},
        {house2, "house2.png"},
        {catus, "catus.png"},
        {scorpion, "scorpion.png"},
        {volcano1, "volcano1.png"},
        {volcano2, "volcano2.png"},
        {frog, "frog.png" },
        {flower, "flower.png" },
        {Back, "back.png"},
        {choosing1, "choosing1.png"},
        {choosing2, "choosing2.png"},
        {choosing3, "choosing3.png"},
        {choosing4, "choosing4.png"},
        {towerdefense, "towerdefense.png"},
        {pyramid, "pyramid.png" },
        {tower1, "tower1.png" },
        {tower2, "tower2.png" },
        {tower3, "tower3.png" },
        {tower4, "tower4.png" },
        {Banner, "banner.png" },
        {Newgame, "newgame.png"},
        {Tutorial, "tutorial.png"},
        {Information, "information.png"},
        {bush1, "bush1.png"},
        {bush2, "bush2.png"},
        {bones1, "bones1.png"},
        {bones2, "bones2.png"},
        {bones3, "bones3.png"},
        {bones4, "bones4.png"},
        {shell1, "shell1.png"},
        {shell2, "shell2.png"},
        {starfish, "starfish.png"},
        {statue1, "statue1.png"},
        {statue2, "statue2.png"},
        {lake, "lake.png"},
        {catus2, "catus2.png"},
        {coral1, "coral1.png"},
        {coral2, "coral2.png"},
        {crab, "crab.png"},
        {stone, "stone.png"},
    };

    for (auto& pair : filenames) {
        TileType type = pair.first;
        std::string file = folderPath + "/" + pair.second;

        sf::Texture texture;
        if (!texture.loadFromFile(file)) {
            //std::cerr << "Không thể tải " << file << "\n";
            continue;

        }
        textures[type] = texture;
    }

    return true;
}

bool TileSet::loadFromFolder(const std::string& folderPath, const std::unordered_set<TileType>& neededTiles)
{
    std::unordered_map<TileType, std::string> filenames = {
        {Grass, "grass.png"},
        {Path, "path.png"},
        {Buildable, "buildable.png"},
        {Castle, "castle.png"},
        {Background, "background.png"},
        {Play, "play.png"},
        {Settings, "settings.png"},
        {Load, "load.png"},
        {Quit, "quit.png"},
        {Map1, "map1.png"},
        {Map2, "map2.png"},
        {Map3, "map3.png"},
        {Map4, "map4.png"},
        {Blank, "blank.png"},
        {Yes, "yes.png"},
        {No, "no.png"},
        {Pause, "pause.png" },
        {Save, "save.png"  },
        {Resume, "resume.png" },
        {Exit, "exit.png" },
        {Restart, "restart.png" },
        {Tree, "tree.png"},
        {house1, "house1.png"},
        {house2, "house2.png"},
        {catus, "catus.png"},
        {scorpion, "scorpion.png"},
        {volcano1, "volcano1.png"},
        {volcano2, "volcano2.png"},
        {frog, "frog.png" },
        {flower, "flower.png" },
        {Back, "back.png"},
        {choosing1, "choosing1.png"},
        {choosing2, "choosing2.png"},
        {choosing3, "choosing3.png"},
        {choosing4, "choosing4.png"},
        {towerdefense, "towerdefense.png"},
        {pyramid, "pyramid.png" },
        {tower1, "tower1.png"},
        {tower2, "tower2.png"},
        {tower3, "tower3.png"},
        {tower4, "tower4.png"},
        {Banner, "banner.png"},
        {Newgame, "newgame.png"},
        {Tutorial, "tutorial.png"},
        {Information, "information.png"},
        {bush1, "bush1.png"},
        {bush2, "bush2.png"},
        {bones1, "bones1.png"},
        {bones2, "bones2.png"},
        {bones3, "bones3.png"},
        {bones4, "bones4.png"},
        {shell1, "shell1.png"},
        {shell2, "shell2.png"},
        {starfish, "starfish.png"},
        {statue1, "statue1.png"},
        {statue2, "statue2.png"},
        {lake, "lake.png"},
        {catus2, "catus2.png"},
        {coral1, "coral1.png"},
        {coral2, "coral2.png"},
        {crab, "crab.png"},
        {stone, "stone.png"},
    };

    for (const auto& pair : filenames) {
        TileType type = pair.first;
        const std::string& filename = pair.second;

        if (neededTiles.find(type) == neededTiles.end()) continue;

        sf::Texture texture;
        if (!texture.loadFromFile(folderPath + "/" + filename)) {
            continue;
            //textures.erase(type);
        }
        textures[type] = texture;
    }
    return true;
}

sf::Texture& TileSet::getTexture(TileType type) {
    if (textures.find(type) == textures.end()) {
        static sf::Texture fallback;
        static bool loaded = fallback.loadFromFile("TileSets/fallback.png");
        return fallback;
    }
    return textures.at(type);
}